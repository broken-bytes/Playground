#include "Atomic.h"
#include <atomic>

uint64_t AtomicInt_FetchAdd(struct AtomicInt* atomic, uint64_t value) {
    return reinterpret_cast<std::atomic<uint64_t>*>(&atomic->counter)->fetch_add(value, std::memory_order_relaxed);
}

uint64_t AtomicInt_FetchSub(struct AtomicInt* atomic, uint64_t value) {
    return reinterpret_cast<std::atomic<uint64_t>*>(&atomic->counter)->fetch_sub(value, std::memory_order_relaxed);
}

uint64_t AtomicInt_Load(struct AtomicInt* atomic) {
    return reinterpret_cast<std::atomic<uint64_t>*>(&atomic->counter)->load(std::memory_order_relaxed);
}

void AtomicInt_Store(struct AtomicInt* atomic, uint64_t value) {
    reinterpret_cast<std::atomic<uint64_t>*>(&atomic->counter)->store(std::memory_order_relaxed);
}
