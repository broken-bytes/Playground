#include "shared/Memory.hxx"

#pragma once

void* operator new[](size_t size, const char* pFile, int line, unsigned int flags, const char* pFunc, int unknown) {
    return ::operator new(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pFile, int line, unsigned int flags, const char* pFunc, int unknown) {
    return ::operator new(size, std::align_val_t(alignment));
}

