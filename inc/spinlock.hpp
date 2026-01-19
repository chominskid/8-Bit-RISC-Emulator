#pragma once

#include <atomic>

// Spinlock that can be acquired as master or slave.
// Slaves will not be able to acquire the lock until all masters release the lock.
class MSSpinLock {
private:
    friend class MSSpinLockGuard;

    std::atomic_uint64_t _master;
    std::atomic_flag _lock;

    void _acquire();
    void _release();

public:
    MSSpinLock();
};

// Lock guard for MSSpinLock.
class MSSpinLockGuard {
public:
    enum class Type {
        SLAVE,
        MASTER
    };

    MSSpinLockGuard();
    MSSpinLockGuard(MSSpinLock& lock, Type type);
    ~MSSpinLockGuard();

    void acquire(MSSpinLock& lock, Type type);
    void release();

private:
    MSSpinLock* _lock;
    Type _type;
};