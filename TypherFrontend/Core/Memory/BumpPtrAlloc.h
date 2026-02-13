#ifndef BUMP_PTR_ALLOC
#define BUMP_PTR_ALLOC

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include "Memory/MemAlloc.h"

template <typename Type, size_t ArraySize = 5>
class SmallArenaVector;

/*
    This should be used on temporary values when we know the size is not going to 
    be large. We do NOT want to use this on trees as they can get unexpectedly
    large.
*/
class BumpPtrAllocator {
    std::uint8_t* base;
    std::size_t   capacity;
    std::size_t   offset;
    uint64_t MaxSize = 0xa00000;
    bool Shared{false};

    static std::size_t align_up(std::size_t n, std::size_t align)
    {
        return (n + align - 1) & ~(align - 1);
    }

public:
    explicit BumpPtrAllocator(std::size_t bytes)
        : base(static_cast<std::uint8_t*>(std::malloc(bytes))),
        capacity(bytes),
        offset(0)
    {
        if (!base) throw std::bad_alloc();
    }

    ~BumpPtrAllocator()
    {
        std::free(base);
    }

    template<typename Type, typename... Args>
    Type* Allocate(Args&&... args)
    {
        void* rawMem = Allocate(sizeof(Type), alignof(Type));
        Type* op = new (rawMem) Type(std::forward<Args>(args)...);
    }

    void* Allocate(size_t size, size_t align = alignof(std::max_align_t)) 
    {
        std::size_t aligned = align_up(offset, align);
        if (aligned + size > capacity) 
            return nullptr;
        void* ptr = base + aligned;
        offset = aligned + size;
        return ptr;
    }

    template<typename T>
    T* Allocate(std::size_t count = 1)
    {
        return static_cast<T*>(
            Allocate(sizeof(T) * count, alignof(T))
        );
    }

    template<typename T>
    T* AllocArray(size_t count) {
        if (count == 0) return nullptr;

        size_t bytes = sizeof(T) * count;
        
        // 1. Calculate Alignment Padding
        uintptr_t current_ptr = (uintptr_t)base + offset;
        uintptr_t alignment = alignof(T);
        uintptr_t padding = (alignment - (current_ptr % alignment)) % alignment;

        if (offset + padding + bytes > capacity) {
            return nullptr; // Out of memory
        }

        // 2. Move offset and return pointer
        T* result = (T*)(base + offset + padding);
        offset += (padding + bytes);

        // 3. Optional: Initialize objects (Placement New)
        for (size_t i = 0; i < count; ++i) {
            new (&result[i]) T(); 
        }

        return result;
    }

    void reset() { 
        offset = 0;
    } // frees all at once

    template<typename Type>
	SmallArenaVector<Type> ArrayAllocate()
	{
		return SmallArenaVector<Type>(*this);
	};
};

template <typename Type, size_t ArraySize>
class SmallArenaVector {
private:
    BumpPtrAllocator& arena;
    Type* data;
    size_t size;
    size_t capacity;

    // The inline "stack" storage
    alignas(Type) char inline_buffer[ArraySize * sizeof(Type)];

    bool is_using_inline_storage() const {
        return data == reinterpret_cast<const Type*>(inline_buffer);
    }

public:
    SmallArenaVector(BumpPtrAllocator& a) 
        : arena(a), data(reinterpret_cast<Type*>(inline_buffer)), size(0), capacity(Size) {}

    void push_back(const Type& value) {
        if (size >= capacity) {
            grow();
        }
        // Using placement new for complex types, 
        // though for PODs (Node*) a simple assignment works.
        new (&data[size++]) Type(value);
    }

    void grow() {
        size_t new_capacity = capacity * 2;
        
        // Allocate the new, larger array in the Arena
        Type* new_data = arena.AllocArray<Type>(new_capacity);

        // Move/Copy existing elements to the Arena
        for (size_t i = 0; i < size; ++i) {
            new (&new_data[i]) Type(std::move(data[i]));
            // No need to call destructors if Type is a pointer (Node*)
            // But for complex types, you'd call data[i].~Type();
        }

        data = new_data;
        capacity = new_capacity;
    }

    // Boilerplate accessors
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