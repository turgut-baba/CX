#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>

class BumpAllocator {
    std::uint8_t* base;
    std::size_t   capacity;
    std::size_t   offset;

    static std::size_t align_up(std::size_t n, std::size_t align) {
        return (n + align - 1) & ~(align - 1);
    }

public:
    explicit BumpAllocator(std::size_t bytes)
        : base(static_cast<std::uint8_t*>(std::malloc(bytes))),
        capacity(bytes),
        offset(0)
    {
        if (!base) throw std::bad_alloc();
    }

    ~BumpAllocator() {
        std::free(base);
    }

    void* allocate(std::size_t size, std::size_t align = alignof(std::max_align_t)) {
        std::size_t aligned = align_up(offset, align);
        if (aligned + size > capacity) return nullptr;
        void* ptr = base + aligned;
        offset = aligned + size;
        return ptr;
    }

    template<typename T>
    T* allocate(std::size_t count = 1) {
        return static_cast<T*>(
            allocate(sizeof(T) * count, alignof(T))
            );
    }

    void reset() { offset = 0; } // frees all at once
};
