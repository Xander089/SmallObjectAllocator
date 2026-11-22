# SmallObjectAllocator

A reproduction of the small-object allocator from Andrei Alexandrescu's *Modern C++ Design*. It makes many small allocations faster and less wasteful than the default `new`.

- **Chunk**: one block of raw memory of equal-size blocks. Each free block stores the index of the next free block, meaning no extra bookkeeping memory is needed.
- **FixedAllocator**: contains a vector of Chunks of a specific size. Method Allocate adds a new one only when all Chunks in the vector are full.
- **SmallObjectAllocator**: the main allocator entry point. It keeps one FixedAllocator per object size and sends each request to the right one. Requests bigger than the maximum size (64 bytes by default) will be forwarded to the global `::operator new`.
- **SmallObject**: any class that inherits from it gets allocated by SmallObjectAllocator. Its `operator delete(void*, size_t)` calls `SmallObjectAllocator::Deallocate`. It uses the sized operator delete version because blocks carry no header, so the allocator needs the compiler to tell it the size in order to find the right FixedAllocator. The destructor is virtual so that this size is correct when you delete through a base pointer.

## Limitations

This implementation is not thread safe. There is one shared allocator, and its caches, its pool of FixedAllocators and each Chunk's free list are changed on every `new` and `delete` with no lock. If two threads allocate or free at the same time, they can take the same block or break the free list, which corrupts memory. Loki's original solves this with a threading-policy template parameter.
