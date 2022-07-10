#include "RingBuffer.h"

namespace Cyclone
{

C_STATUS RingBuffer::Init(uint8* pBegin, uint8* pCurrFree, uint8* pCurrOccupied, uint8* pEnd)
{
    m_pBegin = pBegin;
    m_pCurrFree = pCurrFree;
    m_pCurrOccupied = pCurrOccupied;
    m_pEnd = pEnd;

    return C_STATUS::C_STATUS_OK;
}

void RingBuffer::DeInit()
{
    m_pBegin = nullptr;
    m_pCurrFree = nullptr;
    m_pCurrOccupied = nullptr;
    m_pEnd = nullptr;
}

int RingBuffer::HasFreeMemory(uint8* pNotFreeBlock, uint64 size, uint32 align)
{
    C_UNREFERENCED(pNotFreeBlock);
    // this method support wrap around memory in ring manner
    uint64 begin_range = AlignPow2(reinterpret_cast<uint64>(m_pCurrFree), (uint64)align);
    uint64 end_range = reinterpret_cast<uint64>(m_pCurrOccupied);

    if (begin_range >= end_range)
    {
        if (begin_range + size > reinterpret_cast<uint64>(m_pEnd))
        {
            begin_range = AlignPow2(reinterpret_cast<uint64>(m_pBegin), (uint64)align);
            return begin_range + size <= end_range ? -1 : 0;
        }

        return 1;
    }
    else
    {
        return begin_range + size <= end_range ? 1 : 0;
    }
}

C_STATUS RingBuffer::Allocate(uint64 numBytes, uint32 alignment, uint8** pOutAddress)
{
    // check size overflow
    {
        uint8* begin_aligned = reinterpret_cast<uint8*>(AlignPow2(reinterpret_cast<uint64>(m_pBegin), (uint64)alignment));
        if (begin_aligned + numBytes > m_pEnd)
            return C_STATUS::C_STATUS_INVALID_ARG;
    }

    if (int32 freeMemSide = HasFreeMemory(m_pCurrOccupied, numBytes, alignment))
    {
        // suballocate from buffer
        m_pCurrFree = reinterpret_cast<uint8*>(AlignPow2(reinterpret_cast<uint64>(freeMemSide > 0 ? m_pCurrFree : m_pBegin), (uint64)alignment));

        if (pOutAddress)
            *pOutAddress = m_pCurrFree;

        m_pCurrFree = reinterpret_cast<uint8*>(m_pCurrFree) + numBytes;
    }
    else
    {
        return C_STATUS::C_STATUS_INVALID_ARG;
    }

    return C_STATUS::C_STATUS_OK;
}

}
