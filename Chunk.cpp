#include "Chunk.h"
#include <cassert>

void Chunk::Initialize(std::size_t blockSize, unsigned char blocks)
{
	m_PtrData = new unsigned char[blockSize * blocks];
	m_FirstAvailableBlock = 0;
	m_BlocksAvailableCount = blocks;
	
	// initialize the first byte of each block to point to the next block
	unsigned char* ptr = m_PtrData;
	for (unsigned char i = 0; i != blocks; ptr += blockSize)
	{
		*ptr = ++i;
	}


}

void* Chunk::Allocate(std::size_t blockSize)
{
	if (m_BlocksAvailableCount <= 0) {
		return nullptr;
	} 
	
	// calculate the offset of the first available block and return its pointer
	std::size_t offset = m_FirstAvailableBlock * blockSize;
	unsigned char* ptrNextAvailableBlock = m_PtrData + offset;
	
	// update the m_FirstAvailableBlock to the next available block and decrease the available blocks count
	m_FirstAvailableBlock = *ptrNextAvailableBlock;
	--m_BlocksAvailableCount;
	
	return ptrNextAvailableBlock;
	
}

void Chunk::Deallocate(void* ptr, std::size_t blockSize)
{
	// 1. pre condition: check that the ptr param is within the Chunk's data buffer
	assert(ptr >= m_PtrData); 


	// 2. static cast to unsigned char
	unsigned char* toRelease = static_cast<unsigned char*>(ptr);
	
	// 3. second pre condition check: the ptr param must be aligned to an actual block
	assert((toRelease - m_PtrData) % blockSize == 0);

	// 4. swap: the first available block index is stored in the first byte of the block being release
	// and the m_FirstAvailableBlock is updated to point to the newly released block
	*toRelease = m_FirstAvailableBlock;
	m_FirstAvailableBlock = static_cast<unsigned char>((toRelease - m_PtrData) / blockSize);

	// 5. third check: the m_FirstAvailableBlock must be equal to the index of the block being released
	assert(m_FirstAvailableBlock == (toRelease - m_PtrData) / blockSize);

	// 6. increment the available blocks count
	++m_BlocksAvailableCount;	
}

// frees the Chunk's data buffer and resets the member variabile to their initial state
void Chunk::Release()
{
	delete[] m_PtrData;
	m_PtrData = nullptr;
	m_FirstAvailableBlock = 0;
	m_BlocksAvailableCount = 0;
}