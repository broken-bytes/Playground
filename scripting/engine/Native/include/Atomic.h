#pragma once
#include <stdint.h>
#include "Defines.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AtomicInt {
    uint64_t counter;
};

uint64_t AtomicInt_FetchAdd(struct AtomicInt* atomic, uint64_t value)
    SWIFT_NAME("AtomicInt.fetchAdd(self:value:)");

uint64_t AtomicInt_FetchSub(struct AtomicInt* atomic,uint64_t value)
    SWIFT_NAME("AtomicInt.fetchSub(self:value:)");

uint64_t AtomicInt_Load(struct AtomicInt* atomic)
    SWIFT_NAME("AtomicInt.load(self:)");

void AtomicInt_Store(struct AtomicInt* atomic, uint64_t value)
    SWIFT_NAME("AtomicInt.store(self:value:)");

#ifdef __cplusplus
}
#endif
