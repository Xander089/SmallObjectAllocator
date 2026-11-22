#pragma once

#include "SmallObjectAllocator.h"

#define DEFAULT_CHUNK_SIZE 4096
#define MAX_SMALL_OBJECT_SIZE 64

/** @brief Overrides the new and delete operators to use the SmallObjectAllocator for allocation and deallocation
 */
 template
    <
        std::size_t chunkSize = DEFAULT_CHUNK_SIZE,
        std::size_t maxSmallObjectSize = MAX_SMALL_OBJECT_SIZE
    >
 class SmallObject
{
public:
 
	static void* operator new(std::size_t size) { 
		return SmallObjectAllocator::Instance(chunkSize, maxSmallObjectSize).Allocate(size); 
	}
	
	
	static void operator delete(void* ptr, std::size_t size) { 
		SmallObjectAllocator::Instance(chunkSize, maxSmallObjectSize).Deallocate(ptr, size); 
	}
	
	
	virtual ~SmallObject() = default;
	
};
