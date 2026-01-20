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

    ~MemoryAllocator()
    {
        for (auto& e : entries) {
            e.dtor(e.ptr);
        }
    }
};

#endif