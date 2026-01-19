#include <SFML/System/String.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <thread>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../inc/screen.hpp"
#include "../inc/computer.hpp"
#include "../inc/split.hpp"

using namespace std::chrono_literals;

std::vector<uint8_t> read_binary(const std::string& filename) {
    std::ifstream image(filename, image.binary | image.ate);
    if (!image) {
        std::cerr << "IO error.\n";
        std::exit(EIO);
    }
    std::vector<uint8_t> data(image.tellg());
    image.seekg(image.beg);
    image.read(reinterpret_cast<char*>(data.data()), data.size());
    return data;
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <program binary>\n";
        return EINVAL;
    }

    Computer computer;

    Screen screen(80, 50, 0, 0, 2.0f);
    const size_t screen_memory_size = screen.memory().size();
    
    MemoryDevicePointer memory_interface = new InterfaceDevice(MemoryDevice::Access::READ_WRITE);
    // bootloader rom 0x0000 to 0x00FF
    memory_interface.get<InterfaceDevice>().add_device(0x0000, new BufferMemoryDevice(0x0100, MemoryDevice::Access::READ_ONLY));
    // main memory 0x0100 to before start of screen character memory
    memory_interface.get<InterfaceDevice>().add_device(0x0100, new BufferMemoryDevice(0xFF00 - screen_memory_size, MemoryDevice::Access::READ_WRITE));
    // map screen character memory from end of main memory to end of address space
    memory_interface.get<InterfaceDevice>().add_device(0x10000 - screen_memory_size, &screen.memory());

    computer.attach_memory(memory_interface);
    computer.debug_init();

    memory_interface->debug_write<Endian::LITTLE>(0x0000, read_binary("./programs/DEBUG_BOOTLOADER.bin"));
    memory_interface->debug_write<Endian::BIG>(0x0300, read_binary(argv[1]));

    computer.reset();

    std::atomic_bool exit;

    auto input_thread_worker = [&] () {
        std::vector<std::string> args = { "step" };
        std::cout << "Type \"help\" for a list of commands.\n";

        static const std::unordered_map<std::string, std::function<void()>> COMMANDS {
            { "exit", [&] () {
                exit.store(true, std::memory_order_relaxed);
            }},
            { "step", [&] () {
                int64_t n = 1;
                if (args.size() == 2) {
                    try {
                        n = std::stoll(args[1]);
                    } catch (const std::exception& e) {
                        n = -1;
                    }
                }
                if (args.size() > 2 || n < 0) {
                    std::cerr << "Invalid command.\n";
                    args = { "step" };
                    return;
                }
                computer.step(n);
            }},
            { "stop", [&] () {
                if (args.size() != 1) {
                    std::cerr << "Invalid command.\n";
                    args = { "stop" };
                    return;
                }
                args = { "step" };
                computer.stop();
            }},
            { "run", [&] () {
                double freq = std::numeric_limits<double>::infinity();
                if (args.size() == 2) {
                    try {
                        freq = std::stod(args[1]);
                    } catch (const std::exception& e) {
                        freq = -1.0;
                    }
                }
                if (args.size() > 2 || freq < 0.0) {
                    std::cerr << "Invalid command.\n";
                    args = { "step" };
                    return;
                }
                args = { "stop" };
                computer.run(freq);
            }},
            { "help", [&] () {
                std::cout << "run:    Run the CPU as quickly as possible (with no timing overhead).\n";
                std::cout << "run f:  Run the CPU at a fixed frequency.\n";
                std::cout << "step:   Execute one CPU cycle.\n";
                std::cout << "step n: Execute n CPU cycles as quickly as possible.\n";
                std::cout << "stop:   Stop the CPU if it's running.\n";
                std::cout << "exit:   Close the emulator.\n";
                args = { "step" };
            }},
        };

        while (!exit.load(std::memory_order_relaxed)) {
            std::string line;
            std::cout << "> " << std::flush;
            std::getline(std::cin, line);
            if (std::cin.eof() || std::cin.fail()) {
                exit.store(true, std::memory_order_relaxed);
                break;
            }

            std::vector<std::string> new_args = split(line);
            if (new_args.size() != 0)
                std::swap(args, new_args);

            auto it = COMMANDS.find(args[0]);
            if (it == COMMANDS.end()) {
                std::cerr << "Invalid command.\n";
                std::swap(args, new_args);
            } else {
                it->second();
            }
        }
    };

    std::thread input_thread(input_thread_worker);

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1280 + 600, 800)), "", sf::Style::Default);
    window.setVerticalSyncEnabled(true);
    sf::Font font;
    if (!font.openFromFile("./fonts/Hack-Regular.ttf")) {
        std::cerr << "Failed to open font.\n";
        return 1;
    }
    sf::Text text(font);
    text.setCharacterSize(18);
    text.setPosition({ 1280.0f, 0.0f });

    while (window.isOpen()) {
        if (exit.load(std::memory_order_relaxed))
            goto closed;

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::KeyPressed>()) {
                if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)
                    goto closed;
            } else if (event->is<sf::Event::Closed>()) {
                goto closed;
            } else if (event->is<sf::Event::Resized>()) {
                const auto size = event->getIf<sf::Event::Resized>()->size;
                sf::FloatRect visibleArea(sf::Vector2f{}, sf::Vector2f{(float)size.x, (float)size.y});
                window.setView(sf::View(visibleArea));
            }
        }

        text.setString(computer.debug_state());

        window.clear();
        window.draw(text);
        screen.draw(window);
        window.display();
    }

closed:
    window.close();
    exit.store(true, std::memory_order_relaxed);
    input_thread.join();
}