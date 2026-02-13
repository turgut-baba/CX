#ifndef SLAB_ALLOC_H
#define SLAB_ALLOC_H

#include <sys/mman.h>
#include <iostream>
#include <cstdint>
#include <vector>
#include "Memory/MemAlloc.h"

template <typename Type>
class SlabVector;

class SlabAllocator {
private:
    struct Slab {
        uint8_t* start;
        uint8_t* next;
        uint8_t* end;
        Slab* nextSlab;

        Slab(size_t size) {
            start = (uint8_t*)mmap(0, size, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            next = start;
            end = start + size;
            nextSlab = nullptr;
        }

        ~Slab() {
            munmap(start, end - start);
        }
    };

    Slab* headSlab;
    Slab* currentSlab;
    size_t defaultSlabSize;

public:
    SlabAllocator(size_t slabSize = 64 * 1024) 
        : defaultSlabSize(slabSize) {
        headSlab = new Slab(defaultSlabSize);
        currentSlab = headSlab;
    }

    ~SlabAllocator() {
        Slab* s = headSlab;
        while (s) {
            Slab* next = s->nextSlab;
            delete s;
            s = next;
        }
    }

    template<typename Type, typename... Args>
    Type* Allocate(Args&&... args)
    {
        void* rawMem = Allocate(sizeof(Type), alignof(Type));
        Type* op = new (rawMem) Type(std::forward<Args>(args)...);
    }

    void* Allocate(size_t size, size_t alignment = 8) {
        // 1. Try to allocate in the current slab
        uintptr_t currentAddr = reinterpret_cast<uintptr_t>(currentSlab->next);
        uintptr_t alignedAddr = (currentAddr + (alignment - 1)) & ~(alignment - 1);
        uint8_t* result = reinterpret_cast<uint8_t*>(alignedAddr);

        // 2. If it doesn't fit, allocate a new slab
        if (result + size > currentSlab->end) {
            // If the requested size is bigger than our default, make a custom slab
            size_t newSize = std::max(defaultSlabSize, size + alignment);
            Slab* newSlab = new Slab(newSize);
            
            currentSlab->nextSlab = newSlab;
            currentSlab = newSlab;
            
            // Recalculate result for the new slab
            result = currentSlab->next; 
        }

        currentSlab->next = result + size;
        return result;
    }

    // "Tip" deallocation: Reclaims memory ONLY if it was the last thing allocated
    void Deallocate(void* ptr, size_t size) {
        if (reinterpret_cast<uint8_t*>(ptr) + size == currentSlab->next) {
            currentSlab->next = reinterpret_cast<uint8_t*>(ptr);
        }
    }

    bool isAtTip(void* ptr, size_t size) {
        return (reinterpret_cast<uint8_t*>(ptr) + size == currentSlab->next);
    }

    template<typename Type>
	SlabVector<Type> ArrayAllocate()
	{
		return SlabVector<Type>(*this);
	};
};



template <typename Type>
class SlabVector {
private:
    SlabAllocator& alloc;
    Type* data;
    size_t size;
    size_t capacity;

public:
    SlabVector(SlabAllocator& a) 
        : alloc(a), data(nullptr), size(0), capacity(0) {}

    void push_back(const Type& value) {
        if (size == capacity) {
            size_t oldCapBytes = capacity * sizeof(Type);
            size_t newCapacity = (capacity == 0) ? 4 : capacity * 2;
            size_t newCapBytes = newCapacity * sizeof(Type);

            if (data && alloc.isAtTip(data, oldCapBytes)) {
                // Optimized: Grow in place!
                alloc.Allocate((newCapacity - capacity) * sizeof(Type), alignof(Type));
                capacity = newCapacity;
            } else {
                // Fallback: Allocate new and copy
                Type* newData = (Type*)alloc.Allocate(newCapBytes, alignof(Type));
                if (data) {
                    for (size_t i = 0; i < size; ++i) newData[i] = data[i];
                    // We can't easily "free" the old data if not at tip, 
                    // but the slab chain manages the overhead.
                }
                data = newData;
                capacity = newCapacity;
            }
        }
        data[size++] = value;
    }

    Type& operator[](size_t index) 
    { 
        return data[index];
    }

    size_t Size() const
    { 
        return size;
    }
    
    Type* begin()
    { 
        return data;
    }
    
    Type* end()
    { 
        return data + size;
    }

    bool empty() 
    {
		return size == 0;
	}
};

#endif