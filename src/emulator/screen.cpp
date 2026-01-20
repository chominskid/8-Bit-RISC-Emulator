#include "../../inc/emulator/screen.hpp"

Screen::Screen(unsigned int width, unsigned int height) :
    _memory(new BufferMemoryDevice(std::bit_ceil(width * height * 2), MemoryDevice::Access::READ_WRITE)),
    width(width),
    height(height)
{
}

MemoryDevice& Screen::memory() const {
    return *_memory;
}

void Screen::debug_print(unsigned int x, unsigned int y, const std::string& str) {
    for (unsigned int i = 0, j = y * width + x; i < str.size() && j < width * height; ++i, ++j) {
        memory().write(j, static_cast<uint8_t>(str[i]));
    }
}
