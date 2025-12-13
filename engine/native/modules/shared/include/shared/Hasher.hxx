#pragma once

#include <cstdint>
#include <string_view>

namespace playground::shared {
    inline uint64_t Hash(std::string_view str) {
        const uint64_t FNV_OFFSET_BASIS = 14695981039346656037ull;
        const uint64_t FNV_PRIME = 1099511628211ull;

        uint64_t hash = FNV_OFFSET_BASIS;
        for (char c : str) {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV_PRIME;
        }
        return hash;
    }
}
