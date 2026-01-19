#include "../inc/screen.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

void Screen::draw_char(unsigned int x, unsigned int y, const uint8_t* bitmap, const sf::Color& foreground, const sf::Color& background) {
    for (unsigned int y2 = 0; y2 < CHAR_HEIGHT; ++y2) {
        for (unsigned int x2 = 0; x2 < 8; ++x2) {
            const bool a = (bitmap[y2] >> x2) & 1;
            image.setPixel({x * 8 + x2, y * CHAR_HEIGHT + y2}, a ? foreground : background);
        }
    }
}

void Screen::draw_screen() {
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            const size_t memory_index = (y * width + x) * 2;
            const uint8_t* bitmap = FONT + _memory->read(memory_index).value * CHAR_HEIGHT;
            const uint8_t color_index = _memory->read(memory_index + 1).value;
            draw_char(x, y, bitmap, COLOR[color_index >> 4], COLOR[color_index & 0x0F]);
        }
    }
}

Screen::Screen(unsigned int width, unsigned int height, unsigned int offset_x, unsigned int offset_y, float scale) :
    width(width),
    height(height),
    image({width * 8, height * CHAR_HEIGHT }),
    texture(image),
    sprite(texture),
    _memory(new BufferMemoryDevice(std::bit_ceil(width * height * 2), MemoryDevice::Access::READ_WRITE))
{
    sprite.setPosition({ (float)offset_x, (float)offset_y });
    sprite.setScale({ (float)scale, (float)scale });
}

MemoryDevice& Screen::memory() const {
    return *_memory;
}

void Screen::debug_print(unsigned int x, unsigned int y, const std::string& str) {
    for (unsigned int i = 0, j = y * width + x; i < str.size() && j < width * height; ++i, ++j) {
        memory().write(j, static_cast<uint8_t>(str[i]));
    }
}

void Screen::draw(sf::RenderWindow& window) {
    draw_screen();
    texture.update(image);
    window.draw(sprite);
}