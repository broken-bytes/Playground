#pragma once

#include <cstdint>

void* operator new[](size_t size, const char* pFile, int line, unsigned int flags, const char* pFunc, int unknown);
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pFile, int line, unsigned int flags, const char* pFunc, int unknown);
