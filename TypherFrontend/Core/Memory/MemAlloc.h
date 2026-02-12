#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include "Memory/ArrayAlloc.h"

class MemoryAllocator {
public:
    using Dtor = void (*)(void*);

    struct Entry {
        void* ptr;
        Dtor  dtor;
    };

    std::vector<Entry> entries;

    template<typename Type, typename... Args>
    Type* Allocate(Args&&... args)
    {
        Dtor dtor;

        if constexpr (!std::is_trivially_destructible_v<Type>) {
            dtor = [](void* p) {
                static_cast<Type*>(p)->~Type();
                ::operator delete(p);
                };
        }
        else {
            dtor = [](void* p) {
                ::operator delete(p);
                };
        }

        Type* allocated = new Type(std::forward<Args>(args)...);
        entries.push_back({ allocated, dtor });
        return allocated;
    }

	template<typename Type>
	ArrayAlloc<Type> ArrayAllocate()
	{
		return ArrayAlloc<Type>();
	};

    template<typename T>
    T* AllocArray(size_t count) {
        if (count == 0) return nullptr;

        size_t bytes = sizeof(T) * count;
        
        // 1. Calculate Alignment Padding
        uintptr_t current_ptr = (uintptr_t)buffer + offset;
        uintptr_t alignment = alignof(T);
        uintptr_t padding = (alignment - (current_ptr % alignment)) % alignment;

        if (offset + padding + bytes > size) {
            return nullptr; // Out of memory
        }

        // 2. Move offset and return pointer
        T* result = (T*)(buffer + offset + padding);
        offset += (padding + bytes);

        // 3. Optional: Initialize objects (Placement New)
        for (size_t i = 0; i < count; ++i) {
            new (&result[i]) T(); 
        }

        return result;
    }

    ~MemoryAllocator()
    {
        for (auto& e : entries) {
            e.dtor(e.ptr);
        }
    }
};

#endif