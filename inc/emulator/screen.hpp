#pragma once
#include <string>

#include "memory.hpp"

class Screen {
private:
    MemoryDevicePointer _memory;

public:
    const unsigned int width;
    const unsigned int height;

    Screen() = delete;
    Screen(const Screen&) = delete;
    Screen(Screen&&) = delete;

    Screen(
        unsigned int width,
        unsigned int height
    );

    MemoryDevice& memory() const;
    void debug_print(unsigned int x, unsigned int y, const std::string& str);

    void draw();
};

