#include "../inc/spinlock.hpp"
#include <atomic>

void MSSpinLock::_acquire() {
    while (_lock.test_and_set(std::memory_order_acquire))
        _lock.wait(true, std::memory_order_relaxed);
}

void MSSpinLock::_release() {
    _lock.clear(std::memory_order_release);
    _lock.notify_all();
}

MSSpinLock::MSSpinLock() : _master(false), _lock(false) {}

MSSpinLockGuard::MSSpinLockGuard() : _lock(nullptr) {}

MSSpinLockGuard::MSSpinLockGuard(MSSpinLock& lock, Type type) : _lock(nullptr) {
    acquire(lock, type);
}

MSSpinLockGuard::~MSSpinLockGuard() {
    if (_lock != nullptr)
        release();
}

void MSSpinLockGuard::acquire(MSSpinLock& lock, Type type) {
    if (_lock != nullptr)
        release();
    _type = type;
    _lock = &lock;
    if (_type == Type::MASTER)
        _lock->_master.fetch_add(1, std::memory_order_relaxed);
    else while (uint64_t x = _lock->_master.load(std::memory_order_relaxed))
        _lock->_master.wait(x, std::memory_order_relaxed);
    _lock->_acquire();
}

void MSSpinLockGuard::release() {
    if (_lock == nullptr)
        return;
    _lock->_release();
    if (_type == Type::MASTER) {
        if (_lock->_master.fetch_sub(1, std::memory_order_relaxed) == 1)
            _lock->_master.notify_all();
    }
    _lock = nullptr;
}