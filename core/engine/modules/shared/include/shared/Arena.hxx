#pragma once

#include <EASTL/allocator.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <stdexcept>
#include <exception>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace playground::memory {
    struct IArena {
        virtual void* Allocate(size_t size, size_t align) = 0;
        virtual void Reset() = 0;
        virtual ~IArena() {}
    };

    struct HeapArena : IArena {
        uint8_t* buffer;
        size_t size;
        size_t offset;

        HeapArena(size_t size) : size(size), offset(0) {
            buffer = new uint8_t[size];
        }

        ~HeapArena() override {
            delete[] buffer;
        }

        void* Allocate(size_t sz, size_t alignment = alignof(std::max_align_t)) override {
            size_t current = reinterpret_cast<size_t>(buffer + offset);
            size_t aligned = (current + alignment - 1) & ~(alignment - 1);
            size_t newOffset = aligned - reinterpret_cast<size_t>(buffer) + sz;

            if (newOffset > size) return nullptr;  // out of memory

            void* result = buffer + (aligned - reinterpret_cast<size_t>(buffer));
            offset = newOffset;

            return result;
        }

        void Reset() override { offset = 0; }
    };

    struct VirtualArena : IArena {
        uint8_t* buffer;
        size_t size;
        size_t offset;

        explicit VirtualArena(size_t sizeInBytes) : size(sizeInBytes), offset(0) {
#ifdef _WIN32
            buffer = static_cast<uint8_t*>(::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            if (!buffer) {
                throw std::bad_alloc();
            }
#else
            // TODO: Add mmap implementation here for non-Windows platforms
            throw std::runtime_error("VirtualArena not implemented on this platform");
#endif
        }

        ~VirtualArena() override {
#ifdef _WIN32
            if (buffer) {
                ::VirtualFree(buffer, 0, MEM_RELEASE);
            }
#endif
        }

        void* Allocate(size_t sz, size_t alignment = alignof(std::max_align_t)) override {
            size_t current = reinterpret_cast<size_t>(buffer + offset);
            size_t aligned = (current + alignment - 1) & ~(alignment - 1);
            size_t newOffset = aligned - reinterpret_cast<size_t>(buffer) + sz;

            if (newOffset > size) {
                return nullptr;
            }

            void* result = buffer + (aligned - reinterpret_cast<size_t>(buffer));
            offset = newOffset;

            return result;
        }

        void Reset() override { offset = 0; }
    };

    template <size_t N>
    struct StackArena : IArena {
        alignas(alignof(std::max_align_t)) uint8_t buffer[N];
        size_t offset;

        StackArena() : offset(0) {
        }

        ~StackArena() override {

        }

        void* Allocate(size_t sz, size_t alignment = alignof(std::max_align_t)) override {
            size_t current = reinterpret_cast<size_t>(buffer + offset);
            size_t aligned = (current + alignment - 1) & ~(alignment - 1);
            size_t newOffset = aligned - reinterpret_cast<size_t>(buffer) + sz;

            if (newOffset > N) {
                std::runtime_error("StackArena out of memory");
            }

            void* result = buffer + (aligned - reinterpret_cast<size_t>(buffer));
            offset = newOffset;

            return result;
        }

        void Reset() override { offset = 0; }
    };

    template <typename T>
    class ArenaAllocator {
        static_assert(std::is_base_of<IArena, T>::value, "T must implement IArena");
    public:
        T* arena;

        using this_type = ArenaAllocator<T>;

        ArenaAllocator(T* x, const char* name = EASTL_NAME_VAL("custom allocator")) {
            arena = x;
#if EASTL_NAME_ENABLED
            _name = name ? name : EASTL_ALLOCATOR_DEFAULT_NAME;
#endif
        }

        ArenaAllocator& operator=(const ArenaAllocator& x)
        {
            return *this;
        }

        ~ArenaAllocator() {}


        void* allocate(size_t n, int flags = 0) {
            return arena->Allocate(n);
        }

        void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0) {
            return arena->Allocate(n, alignment);
        }

        void deallocate(void*, size_t) {
            // no-op: arenas don’t deallocate per object
        }

    protected:
#if EASTL_NAME_ENABLED
        const char* _name; // Debug name, used to track memory.

        const char* get_name() const { return _name; }
        void set_name(const char* pName) { this->_name = pName; }
#endif


        ArenaAllocator select_on_container_copy_construction() const { return *this; }
    };


    template <typename T>
    inline bool operator==(const ArenaAllocator<T>& lhs, const ArenaAllocator<T>& rhs) {
        static_assert(std::is_base_of<IArena, T>::value, "T must implement IArena");

        return lhs.arena == rhs.arena;
    }
    template <typename T>
    inline bool operator!=(const ArenaAllocator<T>& lhs, const ArenaAllocator<T>& rhs) {
        static_assert(std::is_base_of<IArena, T>::value, "T must implement IArena");

        return lhs.arena != rhs.arena;
    }
}
