#include "SmallObjectAllocator.h"
#include <cassert>
#include <memory>
#include <algorithm>

SmallObjectAllocator::SmallObjectAllocator(size_t chunkSize, size_t maxObjectSize)
	: m_PtrLastAlloc(nullptr), 
	  m_PtrLastDealloc(nullptr), 
	  m_ChunkSize(chunkSize), 
	  m_MaxObjectSize(maxObjectSize)
{

}

void* SmallObjectAllocator::Allocate(size_t numBytes)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	// fallback
	if (numBytes > m_MaxObjectSize)
	{
		return ::operator new(numBytes);
	}

	// the internal cache matches the requested numBytes: allocate using this cache
	if (m_PtrLastAlloc != nullptr && m_PtrLastAlloc->BlockSize() == numBytes)
	{
		return m_PtrLastAlloc->Allocate();
	}

	Pool::iterator it = std::lower_bound(
		m_Pool.begin(), 
		m_Pool.end(), 
		numBytes, 
		[](const FixedAllocator& fa, std::size_t size) { return fa.BlockSize() < size;} 
	);

    if (it == m_Pool.end() || it->BlockSize() != numBytes)
    {     
		it = m_Pool.insert(it, FixedAllocator(numBytes, m_ChunkSize));
        m_PtrLastDealloc = &*m_Pool.begin();
    }

    m_PtrLastAlloc = &*it;
    return m_PtrLastAlloc->Allocate();

	
}

void SmallObjectAllocator::Deallocate(void* ptr, size_t size)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	// fallback
	if (size > m_MaxObjectSize)
	{
		::operator delete(ptr);
		return;
	}

    if (m_PtrLastDealloc != nullptr && m_PtrLastDealloc->BlockSize() == size)
    {
        m_PtrLastDealloc->Deallocate(ptr);
        return;
    }
    
	Pool::iterator it = std::lower_bound(
		m_Pool.begin(), 
		m_Pool.end(), 
		size, 
        [](const FixedAllocator& fa, std::size_t blockSize) { return fa.BlockSize() < blockSize; });
    
	assert(it != m_Pool.end());
    assert(it->BlockSize() == size);
    m_PtrLastDealloc = &*it;
    m_PtrLastDealloc->Deallocate(ptr);
}
