#include <cassert>

#include "FixedAllocator.h"
#include "Chunk.h"

#include <utility>

#define UCHAR_MAX 255

FixedAllocator::FixedAllocator(std::size_t blockSize, std::size_t chunkSize)
: m_BlockSize(blockSize), m_AllocChunk(nullptr), m_DeallocChunk(nullptr)
{
    assert(m_BlockSize > 0);

    // clamp to 255
    std::size_t numBlocks = chunkSize / blockSize;
    if (numBlocks > UCHAR_MAX) numBlocks = UCHAR_MAX;
    else if (numBlocks == 0) numBlocks = 8 * blockSize;
    
    m_NumBlocks = static_cast<unsigned char>(numBlocks);
    assert(m_NumBlocks == numBlocks);
}

FixedAllocator::FixedAllocator(FixedAllocator &&other) noexcept
    : m_BlockSize(other.m_BlockSize),
      m_NumBlocks(other.m_NumBlocks),
      m_Chunks(std::move(other.m_Chunks)),
      m_AllocChunk(other.m_AllocChunk),
      m_DeallocChunk(other.m_DeallocChunk)
{
    
    other.m_Chunks.clear();
    other.m_AllocChunk = nullptr;
    other.m_DeallocChunk = nullptr;
}

FixedAllocator& FixedAllocator::operator=(FixedAllocator&& other) noexcept
{
    if (this != &other)
    {
        // free this chunks first to prevent memory leaks
        ReleaseAllChunks();

        m_BlockSize = other.m_BlockSize;
        m_NumBlocks = other.m_NumBlocks;
        m_Chunks = std::move(other.m_Chunks);
        
        m_AllocChunk = other.m_AllocChunk;
        m_DeallocChunk = other.m_DeallocChunk;

        other.m_Chunks.clear();
        other.m_AllocChunk = nullptr;
        other.m_DeallocChunk = nullptr;
    }
    return *this;
}

FixedAllocator::~FixedAllocator()
{
    ReleaseAllChunks();
    m_AllocChunk = nullptr;
    m_DeallocChunk = nullptr;
}

void FixedAllocator::ReleaseAllChunks()
{
    for (auto& chunk : m_Chunks)
    {
        chunk.Release();
    }
    m_Chunks.clear();
}

void FixedAllocator::Init(std::size_t blockSize, unsigned char numBlocks)
    {
        m_BlockSize = blockSize;
        m_NumBlocks = numBlocks;
        
        // prevents leaking chunks
        ReleaseAllChunks(); 
        
        m_AllocChunk = nullptr;
        m_DeallocChunk = nullptr;
    }


void* FixedAllocator::Allocate()
{
    //1. If memory still available --> allocate immediately
    
    bool memoryAvailable = m_AllocChunk != nullptr && m_AllocChunk->m_BlocksAvailableCount > 0;
    
    if (memoryAvailable) {
        return m_AllocChunk->Allocate(m_BlockSize);
    }
    
    // 2. Search for an available Chunk    
    bool availableChunkFound = false;
    
    for (auto& chunk : m_Chunks) {
        if (chunk.m_BlocksAvailableCount > 0)
            {
                availableChunkFound = true;
                m_AllocChunk = &chunk;
                break;
            }
        }

    // 3. If not any available Chunk --> create a new one
    if (!availableChunkFound) {

        // 3.1 augment vector capacity
        m_Chunks.reserve(m_Chunks.size() + 1);

        // 3.2 create and init a new Chunk
        Chunk newChunk{};
        newChunk.Initialize(m_BlockSize, m_NumBlocks);

        // 3.3 push back the chunk
        // Can't use emplace back because no explicit ctor was defined to ensure the exact match of the Init(...) method
        m_Chunks.push_back(newChunk);

        // 3.4 update FixedAllocator member variable pointers: 
        // This way we increase the likelihood of a fast allocation next time
        m_AllocChunk = &m_Chunks.back();
        m_DeallocChunk = &m_Chunks.back();

    }
    
    // 4. check that the newly created Chunk is valid
    assert(m_AllocChunk != nullptr);
    assert(m_AllocChunk->m_BlocksAvailableCount > 0);

    return m_AllocChunk->Allocate(m_BlockSize);
 }

void FixedAllocator::Deallocate(void* ptr)
{
    assert(ptr != nullptr);
    
    // 0. Cache validity check: Deallocate cache must be a valid pointer and lie within the Chunks vector range
    assert(m_DeallocChunk != nullptr);
    assert(!m_Chunks.empty());
    assert(&m_Chunks.front() <= m_DeallocChunk);
    assert(&m_Chunks.back() >= m_DeallocChunk);
    

    // 1. Try to find the closest Chunk containing the pointer param "ptr", starting from deallocChunk_
    m_DeallocChunk = VicinityFind(ptr);
    
    // 2. found container validity check: param ptr must lie within the the found Chunk
    assert(m_DeallocChunk != nullptr);
    assert(ptr >= m_DeallocChunk->m_PtrData);
    assert(ptr < m_DeallocChunk->m_PtrData + m_NumBlocks * m_BlockSize);

    // 3. Deallocate the Chunk's block at ptr
    m_DeallocChunk->Deallocate(ptr, m_BlockSize);

    // 4. Try to move the deallocChunk_ at the end of the vector if all its blocks ara available
    ReleaseEmptyChunks();
}

Chunk* FixedAllocator::VicinityFind(void* ptr)
{

    const std::size_t chunkSize = m_NumBlocks * m_BlockSize;

    Chunk* low = m_DeallocChunk;
    Chunk* high = m_DeallocChunk + 1;
    Chunk* lowerBound = &m_Chunks.front();
    Chunk* higherBound = &m_Chunks.back() + 1;

    // search ptr's container Chunk starting from deallocChunk_ - both towards left and right direction
    bool outOfBounds = false;
    while(!outOfBounds)
    {
        if (low != nullptr)
        {
            
            bool lowContainsPtrParam = ptr >= low->m_PtrData && ptr < low->m_PtrData + chunkSize;
            if (lowContainsPtrParam)
            {
                return low; // Chunk container found
            }
            
            if (low == lowerBound) { low = nullptr; }
            else { --low; }
        }

        if (high < higherBound)
        {
            bool highContainsPtrParam = ptr >= high->m_PtrData && ptr < high->m_PtrData + chunkSize;
            if (highContainsPtrParam)
            {
                return high; // Chunk container found
            }
            ++high;
        } 
        else if (low == nullptr) { 
            // if both high and low are out of bounds --> ptr does not lie within the fixed allocator
            outOfBounds = true;
        }  
    }

    assert(false); // invalid ptr param
    return nullptr;
}

void FixedAllocator::ReleaseEmptyChunks()
{
    bool deallocChunkEmpty = m_DeallocChunk->m_BlocksAvailableCount == m_NumBlocks;
    if (deallocChunkEmpty)
        {
        Chunk& lastChunk = m_Chunks.back();

        // if deallocChunk_ is the last Chunk of the vector:
        //  - check if the second-last's blocks are all available as well
        if (&lastChunk == m_DeallocChunk)
            {
            bool secondLastChunkBlocksAllAvailable = m_Chunks.size() > 1 && m_Chunks[m_Chunks.size() - 2].m_BlocksAvailableCount == m_NumBlocks;
            if (secondLastChunkBlocksAllAvailable)
                {
                // Two empty Chunks at the end of the vector: let's release the last and pop it
                lastChunk.Release();
                m_Chunks.pop_back();

                // Reset pointers to a guaranteed valid chunk to prevent dangling pointers after pop_back()
                m_AllocChunk = m_DeallocChunk = &m_Chunks.front();
                }
            return;
            }


        // if the last Chunk is all avalable, let's release and pop it, then update the caches to the vector's front
        bool lastChunkEmpty = lastChunk.m_BlocksAvailableCount == m_NumBlocks;
        if (lastChunkEmpty)
        {

            lastChunk.Release();
            m_Chunks.pop_back();

            // Reset pointers to a guaranteed valid chunk to prevent dangling pointers after pop_back()
            m_AllocChunk = m_DeallocChunk = &m_Chunks.front();
        } else
        {
            // else Swap empty chunk to the back as a reserve; deallocChunk_ now safely points to the swapped active chunk
            std::swap<Chunk>(*m_DeallocChunk, lastChunk);
            // and point allocChunk_ to the newly emptied reserve chunk at the back for fast O(1) future allocations
            m_AllocChunk = &m_Chunks.back();
        }
    }
}