#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include "Memory/ArrayAlloc.h"
#include "Memory/SlabAlloc.h"
#include "Memory/BumpPtrAlloc.h"
#include <memory>

/*
    This should be used when there is a destructor.
    It's not reccomended unless absolutely needed.
*/
class DtorMemAllocator {
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

    ~DtorMemAllocator()
    {
        for (auto& e : entries) {
            e.dtor(e.ptr);
        }
    }
};

enum AllocatorType {
    DTOR,
    SLAB,
    BUMP
};

template <AllocatorType K> struct AllocTypeMap;

template <> struct AllocTypeMap<AllocatorType::DTOR> { using type = DtorMemAllocator; };
template <> struct AllocTypeMap<AllocatorType::SLAB> { using type = SlabAllocator; };
template <> struct AllocTypeMap<AllocatorType::BUMP> { using type = BumpPtrAllocator; };

struct MemoryAllocator {
    std::shared_ptr<DtorMemAllocator> dtorAlloc; // Includes Dtors
    std::shared_ptr<SlabAllocator> slabAlloc; // Versatile
    std::shared_ptr<BumpPtrAllocator> bumpAlloc; // Faster
};

#endif