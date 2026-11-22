#pragma once

#include <cstddef>

// A plain old data object representing a raw block of memory with a fixed number of blocks of a given size. 
struct Chunk {


	void Initialize(std::size_t blockSize, unsigned char blocks);
	void* Allocate(std::size_t blockSize);
	void Deallocate(void* ptr, std::size_t blockSize);
	void Release();

	unsigned char* m_PtrData; // the raw memory block	
	unsigned char m_FirstAvailableBlock; 
	unsigned char m_BlocksAvailableCount; 

};