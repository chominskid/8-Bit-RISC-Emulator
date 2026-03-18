#pragma once

#include <concepts>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class MemoryMap {
private:
    std::unordered_map<size_t, std::vector<uint8_t>> _map;
    typename decltype(_map)::iterator _curr;

public:
    MemoryMap();

    void set_address(size_t address);
    size_t curr_addr() const;
    size_t curr_size() const;
    void push_byte(uint8_t x);

    template <std::forward_iterator It>
    requires std::convertible_to<std::iter_value_t<It>, uint8_t>
    void append(It begin, It end) {
        for (It it = begin; it != end; ++it) {
            push_byte(*it);
        }
    }

    void write(const std::string& filename) const;
};