#include "../inc/memory.hpp"

MemoryResult::MemoryResult(uint8_t value) :
    signal(Signal::SUCCESS),
    value(value)
{}
MemoryResult::MemoryResult(Signal signal, uint8_t value) :
    signal(signal),
    value(value)
{}



MemoryDevice::MemoryDevice(Access access) :
    ref_count(0),
    access(access)
{}

MemoryDevice::~MemoryDevice() = default;

size_t MemoryDevice::size() const {
    return 0;
}

void MemoryDevice::debug_write(size_t, uint8_t) {

}

MemoryResult MemoryDevice::write(size_t, uint8_t) {
    return { MemoryResult::Signal::CANNOT_WRITE };
}

MemoryResult MemoryDevice::read(size_t) const {
    return { MemoryResult::Signal::CANNOT_READ };
}

void MemoryDevice::debug_fill(size_t size, uint8_t value) {
    for (size_t i = 0; i < size; ++i)
        debug_write(i, value);
}



void MemoryDevicePointer::add_ref() {
    if (device)
        device->ref_count++;
}

void MemoryDevicePointer::del_ref() {
    if (device) {
        device->ref_count--;
        if (device->ref_count == 0)
            delete device;
    }
}

MemoryDevicePointer::MemoryDevicePointer(MemoryDevice* device) :
    device(device)
{
    add_ref();
}
MemoryDevicePointer::MemoryDevicePointer(const MemoryDevicePointer& other) :
    device(other.device)
{
    add_ref();
}
MemoryDevicePointer::MemoryDevicePointer(MemoryDevicePointer&& other) :
    device(other.device)
{
    other.device = nullptr;
}
MemoryDevicePointer::~MemoryDevicePointer() {
    del_ref();
}

void MemoryDevicePointer::clear() {
    del_ref();
    device = nullptr;
}
MemoryDevicePointer& MemoryDevicePointer::operator=(MemoryDevice* device) {
    del_ref();
    this->device = device;
    add_ref();
    return *this;
}
MemoryDevicePointer& MemoryDevicePointer::operator=(const MemoryDevicePointer& other) {
    del_ref();
    device = other.device;
    add_ref();
    return *this;
}
MemoryDevicePointer& MemoryDevicePointer::operator=(MemoryDevicePointer&& other) {
    del_ref();
    device = other.device;
    other.device = nullptr;
    return *this;
}

MemoryDevicePointer::operator bool() const {
    return device != nullptr;
}
MemoryDevice* MemoryDevicePointer::get() const {
    return device;
}
MemoryDevice& MemoryDevicePointer::operator*() const {
    return *device;
}
MemoryDevice* MemoryDevicePointer::operator->() const {
    return device;
}



InterfaceDevice::Entry::Entry(size_t address, MemoryDevice* device) :
    address(address),
    device(device)
{}

bool InterfaceDevice::Entry::operator<(size_t address) const {
    return this->address < address;
}

const InterfaceDevice::Entry* InterfaceDevice::resolve_address(size_t address) const {
    if (table.size() == 0)
        return nullptr;
    auto it = std::lower_bound(table.begin(), table.end(), address);
    if (it != table.end() && it->address == address)
        return &*it;
    else if (it == table.begin())
        return nullptr;
    else
        return &*std::prev(it);
}

InterfaceDevice::InterfaceDevice(Access access) :
    MemoryDevice(access)
{}

void InterfaceDevice::add_device(size_t address, MemoryDevice* device) {
    auto it = std::lower_bound(table.begin(), table.end(), address);
    if (it != table.end() && it->address == address)
        throw std::invalid_argument("MemoryInterfaceDevice: address is already mapped");
    table.emplace(it, address, device);
}
void InterfaceDevice::add_device(size_t address, const MemoryDevicePointer& device) {
    add_device(address, device.get());
}

size_t InterfaceDevice::size() const {
    if (table.size() == 0)
        return 0;
    return table.back().address + table.back().device->size();
}

void InterfaceDevice::debug_write(size_t address, uint8_t value) {
    const Entry* entry = resolve_address(address);
    if (entry == nullptr)
        return;
    entry->device->debug_write(address - entry->address, value);
}

MemoryResult InterfaceDevice::read(size_t address) const {
    if (!((int)access & (int)Access::READ_ONLY))
        return { MemoryResult::Signal::CANNOT_READ };
    const Entry* entry = resolve_address(address);
    if (entry == nullptr)
        return { MemoryResult::Signal::OUT_OF_RANGE };
    return entry->device->read(address - entry->address);
}

MemoryResult InterfaceDevice::write(size_t address, uint8_t value) {
    if (!((int)access & (int)Access::WRITE_ONLY))
        return { MemoryResult::Signal::CANNOT_WRITE };
    const Entry* entry = resolve_address(address);
    if (entry == nullptr)
        return { MemoryResult::Signal::OUT_OF_RANGE };
    return entry->device->write(address - entry->address, value);
}



BufferMemoryDevice::BufferMemoryDevice(size_t size, Access access) :
    MemoryDevice(access),
    _size(size),
    data(new uint8_t[size])
{
    std::fill_n(&data[0], _size, 0);
}

size_t BufferMemoryDevice::size() const {
    return _size;
}

void BufferMemoryDevice::debug_write(size_t address, uint8_t value) {
    if (address >= _size)
        return;
    MSSpinLockGuard guard(lock, MSSpinLockGuard::Type::SLAVE);
    data[address] = value;
}

MemoryResult BufferMemoryDevice::read(size_t address) const {
    if (!((int)access & (int)Access::READ_ONLY))
        return { MemoryResult::Signal::CANNOT_READ };
    if (address >= _size)
        return { MemoryResult::Signal::OUT_OF_RANGE };
    MSSpinLockGuard guard(lock, MSSpinLockGuard::Type::SLAVE);
    const uint8_t value = data[address];
    // std::cout << "Read " << +value << " from address " << address << '\n';
    return value;
}

MemoryResult BufferMemoryDevice::write(size_t address, uint8_t value) {
    if (!((int)access & (int)Access::WRITE_ONLY))
        return { MemoryResult::Signal::CANNOT_WRITE };
    if (address >= _size)
        return { MemoryResult::Signal::OUT_OF_RANGE };
    MSSpinLockGuard guard(lock, MSSpinLockGuard::Type::SLAVE);
    data[address] = value;
    // std::cout << "Wrote "  << +value << " to address " << address << '\n';
    return {};
}