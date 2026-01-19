#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <ranges>
#include <vector>

#include "spinlock.hpp"

enum class Endian {
    LITTLE,
    BIG
};

class MemoryResult {
public:
    enum class Signal : uint8_t {
        SUCCESS,
        WAIT,
        OUT_OF_RANGE,
        CANNOT_READ,
        CANNOT_WRITE,
    };

    Signal signal;
    uint8_t value;

    MemoryResult(uint8_t value = 0);
    MemoryResult(Signal signal, uint8_t value = 0);
};

class MemoryDevice {
private:
    friend class MemoryDevicePointer;
    uint64_t ref_count;

public:
    enum class Access : uint8_t {
        READ_ONLY = 0x01,
        WRITE_ONLY = 0x02,
        READ_WRITE = READ_ONLY | WRITE_ONLY,
    };

    Access access;

    MemoryDevice(Access access);
    virtual ~MemoryDevice();

    virtual size_t size() const;
    virtual void debug_write(size_t address, uint8_t value);
    virtual MemoryResult write(size_t address, uint8_t value);
    virtual MemoryResult read(size_t address) const;

    void debug_fill(size_t size, uint8_t value);

    template <Endian E, std::forward_iterator It>
    requires std::integral<std::remove_cvref_t<std::iter_value_t<It>>>
    void debug_write(size_t address, It begin, It end) {
        using T = std::make_unsigned_t<std::remove_cvref_t<std::iter_value_t<It>>>;
        const size_t n = std::numeric_limits<T>::digits / 8;
        const size_t size = this->size();
        for (auto it = begin; it != end; ++it) {
            if (address + n >= size) {
                std::cerr << "MemoryDevice::debug_write(): warning: data truncated, past end of memory.\n";
                return;
            }

            const T x = *it;
            for (size_t i = 0; i < n; ++i) {
                if constexpr (E == Endian::LITTLE)
                    debug_write(address + i, x >> 8 * i);
                else
                    debug_write(address + n - i - 1, x >> 8 * i);
            }

            address += n;
        }
    }

    template <Endian E, std::ranges::forward_range R>
    requires std::integral<std::remove_cvref_t<std::ranges::range_value_t<R>>>
    void debug_write(size_t address, const R& range) {
        debug_write<E>(address, std::ranges::cbegin(range), std::ranges::cend(range));
    }
};

class MemoryDevicePointer {
private:
    MemoryDevice* device;

    void add_ref();
    void del_ref();

public:
    MemoryDevicePointer(MemoryDevice* device = nullptr);
    MemoryDevicePointer(const MemoryDevicePointer& other);
    MemoryDevicePointer(MemoryDevicePointer&& other);
    ~MemoryDevicePointer();

    void clear();
    MemoryDevicePointer& operator=(MemoryDevice* device);
    MemoryDevicePointer& operator=(const MemoryDevicePointer& other);
    MemoryDevicePointer& operator=(MemoryDevicePointer&& other);

    operator bool() const;
    MemoryDevice* get() const;
    MemoryDevice& operator*() const;
    MemoryDevice* operator->() const;

    template <typename T>
    T& get() {
        return *static_cast<T*>(device);
    }
    template <typename T>
    const T& get() const {
        return *static_cast<T*>(device);
    }
};

// Memory device that can map memory addresses to other memory devices.
class InterfaceDevice : public MemoryDevice {
private:
    struct Entry {
        size_t address;
        MemoryDevicePointer device;

        Entry(size_t address, MemoryDevice* device);

        bool operator<(size_t address) const;
    };

    std::vector<Entry> table;

    const Entry* resolve_address(size_t address) const;

public:
    InterfaceDevice(Access access);

    void add_device(size_t address, MemoryDevice* device);
    void add_device(size_t address, const MemoryDevicePointer& device);

    size_t size() const override;
    void debug_write(size_t address, uint8_t value) override;
    MemoryResult read(size_t address) const override;
    MemoryResult write(size_t address, uint8_t value) override;
};

class BufferMemoryDevice : public MemoryDevice {
protected:
    const size_t _size;
    std::unique_ptr<uint8_t[]> data;
    mutable MSSpinLock lock;

public:
    BufferMemoryDevice(size_t size, Access access);

    size_t size() const override;
    void debug_write(size_t address, uint8_t value) override;
    MemoryResult read(size_t address) const override;
    MemoryResult write(size_t address, uint8_t value) override;
};