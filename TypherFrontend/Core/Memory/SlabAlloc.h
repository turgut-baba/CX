#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <algorithm>

class SlabArena {
    struct Slab {
        Slab* next;
        std::size_t offset;
        std::size_t capacity;
        std::uint8_t data[1]; // flexible array
    };

    Slab* head;
    Slab* current;
    std::size_t slab_size;

    static std::size_t align_up(std::size_t n, std::size_t align) {
        return (n + align - 1) & ~(align - 1);
    }

    Slab* new_slab(std::size_t min_capacity) {
        std::size_t cap = std::max(slab_size, min_capacity);
        std::size_t total =
            sizeof(Slab) - 1 + cap;

        Slab* s = static_cast<Slab*>(std::malloc(total));
        if (!s) throw std::bad_alloc();

        s->next = nullptr;
        s->offset = 0;
        s->capacity = cap;
        return s;
    }

public:
    explicit SlabArena(std::size_t slabSize = 64 * 1024)
        : head(nullptr), current(nullptr), slab_size(slabSize) {
    }

    ~SlabArena() {
        reset(true);
    }

    void* allocate(std::size_t size,
        std::size_t align = alignof(std::max_align_t)) {
        if (!current) {
            head = current = new_slab(size + align);
        }

        std::size_t aligned = align_up(current->offset, align);

        if (aligned + size > current->capacity) {
            Slab* s = new_slab(size + align);
            current->next = s;
            current = s;
            aligned = align_up(current->offset, align);
        }

        void* ptr = current->data + aligned;
        current->offset = aligned + size;
        return ptr;
    }

    template<typename T>
    T* allocate(std::size_t count = 1) {
        return static_cast<T*>(
            allocate(sizeof(T) * count, alignof(T))
            );
    }

    // reset arena; optionally free slabs
    void reset(bool free_slabs = false) {
        if (!head) return;

        if (free_slabs) {
            Slab* s = head;
            while (s) {
                Slab* next = s->next;
                std::free(s);
                s = next;
            }
            head = current = nullptr;
        }
        else {
            for (Slab* s = head; s; s = s->next)
                s->offset = 0;
            current = head;
        }
    }
};

