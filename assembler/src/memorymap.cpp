#include "../inc/memorymap.hpp"
#include <fstream>
#include <limits>
#include <type_traits>

template <typename T>
void bin_write(std::ofstream& file, T x) {
    using U = std::make_unsigned_t<T>;
    static constexpr size_t N = std::numeric_limits<U>::digits;
    static constexpr bool CAN_WRITE_DIRECT = N == sizeof(T) && std::endian::native == std::endian::little;

    if constexpr (CAN_WRITE_DIRECT) {
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
    static constexpr bool CAN_READ_DIRECT = N == sizeof(T) && std::endian::native == std::endian::little;

    if constexpr (CAN_READ_DIRECT) {
        U x;
        file.read(reinterpret_cast<char*>(&x), sizeof(U));
        return x;
    } else {
        U x = 0;
        
    }
}

MemoryMap::MemoryMap() {
    set_address(0);
}

void MemoryMap::set_address(size_t address) {
    _curr = _map.emplace(address, std::vector<uint8_t>()).first;
}

size_t MemoryMap::curr_addr() const {
    return _curr->first;
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
        bin_write(file, section.first);
        bin_write(file, section.second.size());
        file.write(reinterpret_cast<const char*>(section.second.data()), section.second.size());
    }
}