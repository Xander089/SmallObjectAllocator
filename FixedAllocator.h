#pragma once

#include <vector>
#include "Chunk.h"


using Chunks = std::vector<Chunk>;

// A fixed-size memory allocator that manages multiple chunks of equal-sized blocks. 
class FixedAllocator
{
	public:

	FixedAllocator() = default;
	explicit FixedAllocator(std::size_t blockSize, std::size_t chunkSize);
	
	// Non-copyable
	FixedAllocator(const FixedAllocator& other) = delete;
	FixedAllocator& operator=(const FixedAllocator& other) = delete; 

	// Moveable
	FixedAllocator(FixedAllocator&& other) noexcept;
	FixedAllocator& operator=(FixedAllocator&& other) noexcept;

	~FixedAllocator();


	void Init(std::size_t blockSize, unsigned char numBlocks);
	void* Allocate();
	void Deallocate(void* ptr);
	
	
	std::size_t BlockSize() const { return m_BlockSize; }

	private:
	
	Chunk* VicinityFind(void* ptr);
	void ReleaseEmptyChunks();	
	void ReleaseAllChunks();

	size_t m_BlockSize = 0;
	unsigned char m_NumBlocks = 0;
	Chunks m_Chunks;
	Chunk* m_AllocChunk = nullptr;
	Chunk* m_DeallocChunk = nullptr;
};	