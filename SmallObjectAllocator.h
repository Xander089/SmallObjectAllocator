// SmallObjectAllocator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vector>
#include <mutex>

#include "FixedAllocator.h"

using Pool = std::vector<FixedAllocator>;

// A memory allocator for small objects that uses FixedAllocator instances internally.
class SmallObjectAllocator
{
    public:

    static SmallObjectAllocator& Instance(std::size_t chunkSize = 4096, std::size_t maxObjectSize = 64)
    {
        static SmallObjectAllocator instance(chunkSize, maxObjectSize);
        return instance;
    }

    // Non-copyable and non-assignable to prevent accidental copying of the allocator, which could lead to double-free errors or memory leaks
    SmallObjectAllocator(const SmallObjectAllocator&) = delete;
    SmallObjectAllocator& operator=(const SmallObjectAllocator&) = delete;

    
    void* Allocate(std::size_t numBytes);
    void Deallocate(void* ptr, std::size_t size);

    private:

    SmallObjectAllocator(std::size_t chunkSize = 4096, std::size_t maxObjectSize = 64);
    
    std::vector<FixedAllocator> m_Pool;

    // internal cache
    FixedAllocator* m_PtrLastAlloc = nullptr;
    FixedAllocator* m_PtrLastDealloc = nullptr;

    // default Chunk size
    std::size_t m_ChunkSize = 4096; 
    
    // default max object size
    std::size_t m_MaxObjectSize = 64; 

	std::mutex m_mutex;
    
};