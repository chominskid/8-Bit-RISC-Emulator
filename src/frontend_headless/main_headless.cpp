#include <atomic>
#include <cstdint>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <thread>

#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../inc/emulator/computer.hpp"
#include "../../inc/emulator/screen.hpp"
#include "../../inc/utils/split.hpp"
#include "../../inc/utils/arg_parse.hpp"

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

static const char *TERM_FG_COLORS[] {
    "30", // 0: BLACK 
    "97", // 1: WHITE 
    "91", // 2: RED 
    "93", // 3: ORANGE (yellow)
    "92", // 4: YELLOW (green)
    "94", // 5: GREEN (blue)
    "96", // 6: BLUE (cyan)
    "95", // 7: PURPLE (magenta)
    "31", // 8: DARK RED 
    "33", // 9: DARK ORANGE (yellow)
    "32", // A: DARK YELLOW (green)
    "34", // B: DARK GREEN (blue)
    "36", // C: DARK BLUE (cyan)
    "35", // D: DARK PURPLE (magenta)
    "37", // E: GRAY 
    "90"  // F: DARK GRAY
};
static const char *TERM_BG_COLORS[] {
    "40",  // 0: BLACK 
    "107", // 1: WHITE 
    "101", // 2: RED 
    "103", // 3: ORANGE (yellow)
    "102", // 4: YELLOW (green)
    "104", // 5: GREEN (blue)
    "106", // 6: BLUE (cyan)
    "105", // 7: PURPLE (magenta)
    "41",  // 8: DARK RED 
    "43",  // 9: DARK ORANGE (yellow)
    "42",  // A: DARK YELLOW (green)
    "44",  // B: DARK GREEN (blue)
    "46",  // C: DARK BLUE (cyan)
    "45",  // D: DARK PURPLE (magenta)
    "47",  // E: GRAY 
    "100"  // F: DARK GRAY
};

void print_color_escape_sequence(const uint8_t color_index) {
    unsigned char foreground = color_index >> 4;
    unsigned char background = color_index & 0x0F;
    std::cout << "\033[" << TERM_FG_COLORS[foreground] << ";" << TERM_BG_COLORS[background] << "m";
}

void print_screen(Screen &screen) {
    auto& memory = screen.memory();
    auto last_color_index = memory.read(1).value;
    print_color_escape_sequence(last_color_index);

    for (unsigned int y = 0; y < screen.height; ++y) {
        for (unsigned int x = 0; x < screen.width; ++x) {
            const size_t memory_index = (y * screen.width + x) * 2;
            const char charcode = memory.read(memory_index).value;

            if (charcode == 0) { continue; }

            const uint8_t color_index = memory.read(memory_index + 1).value;

            if (color_index != last_color_index) {
                print_color_escape_sequence(color_index);
            }
            
            std::cout << charcode;
        }
    }

    std::cout << "\033[0m" << std::endl;  // reset formatting
}

int main(int argc, const char* argv[]) {
    ArgParse args(argc, argv);

    if (auto error = args.get_error()) {
        std::cerr << "Error parsing arguments: " << *error << std::endl;
        return EINVAL;
    }

    auto step_limit_str = args.take_option("--step-limit");
    auto program_file = args.take_normal();

    if (args.has_remaining() || !program_file.has_value()) {
        std::cerr << "Usage: " << argv[0] << " <program binary> [--step-limit n]" << std::endl;
        return EINVAL;
    }

    Computer computer;

    Screen screen(80, 50);
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
    memory_interface->debug_write<Endian::BIG>(0x0300, read_binary(*program_file));

    computer.reset();

    int step_limit = step_limit_str.has_value() ? std::stoi(*step_limit_str) : 10000;
    computer.step_sync(step_limit);

    print_screen(screen);
}