#include "../inc/memorymap.hpp"
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

template <typename T>
void bin_write(std::ofstream& file, T x) {
    using U = std::make_unsigned_t<T>;
    static constexpr size_t N = std::numeric_limits<U>::digits;

    if constexpr (std::endian::native == std::endian::little) {
        file.write(reinterpret_cast<const char*>(&x), sizeof(T));
    } else {
        U _x = x;
        for (size_t i = 0; i < sizeof(U); ++i) {
            const unsigned char byte = _x;
            file.write(reinterpret_cast<const char*>(&byte), 1);
            _x >>= 8;
        }
    }
}

template <typename T>
T bin_read(std::ifstream& file) {
    using U = std::make_unsigned_t<T>;
    static constexpr size_t N = std::numeric_limits<U>::digits;

    if constexpr (std::endian::native == std::endian::little) {
        U x;
        file.read(reinterpret_cast<char*>(&x), sizeof(U));
        return x;
    } else {
        U x = 0;
        for (size_t i = 0; i < sizeof(U); ++i) {
            x <<= 8;
            int c = file.get();
            if (c == std::char_traits<char>::eof())
                throw std::runtime_error("IO error.");
            x |= (unsigned char)c;
        }
        return x;
    }
}

MemoryMap::MemoryMap() {
    set_address(0);
}

MemoryMap::const_iterator MemoryMap::begin() const {
    return _map.begin();
}

MemoryMap::const_iterator MemoryMap::end() const {
    return _map.end();
}

void MemoryMap::set_address(size_t address) {
    _curr = _map.emplace(address, std::vector<uint8_t>()).first;
}

size_t MemoryMap::curr_addr() const {
    return _curr->first + curr_size();
}

size_t MemoryMap::curr_size() const {
    return _curr->second.size();
}

void MemoryMap::push_byte(uint8_t x) {
    _curr->second.push_back(x);
}

void MemoryMap::write(const std::string& filename) const {
    std::ofstream file(filename, file.binary);
    for (const auto& section: _map) {
        if (section.second.size() == 0)
            continue;
        bin_write<uint64_t>(file, section.first);
        bin_write<uint64_t>(file, section.second.size());
        file.write(reinterpret_cast<const char*>(section.second.data()), section.second.size());
    }
}

void MemoryMap::read(const std::string& filename) {
    std::ifstream file(filename, file.binary);
    for (;;) {
        const size_t address = bin_read<uint64_t>(file);
        const size_t size = bin_read<uint64_t>(file);
        if (file.eof())
            break;
        std::cout << "address: " << address << ", bytes: " << size << '\n';
        set_address(address);
        _curr->second.resize(size);
        file.read(reinterpret_cast<char*>(_curr->second.data()), size);
    }
}