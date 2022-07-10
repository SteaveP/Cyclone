#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

class RingBuffer
{
public:
    C_STATUS Allocate(uint64 numBytes, uint32 alignment, uint8** pOutAddress);

    C_STATUS Init(uint8* pBegin, uint8* pCurrFree, uint8* pCurrOccupied, uint8* pEnd);
    void DeInit();

    void SetCurrOccupied(uint8* pOccupied) { m_pCurrOccupied = pOccupied; }

    const uint8* GetBegin() const { return m_pBegin; }
    uint8* GetBegin() { return m_pBegin; }

    const uint8* GetCurrFree() const { return m_pCurrFree; }
    uint8* GetCurrFree() { return m_pCurrFree; }

    const uint8* GetCurrOccupied() const { return m_pCurrOccupied; }
    uint8* GetCurrOccupied() { return m_pCurrOccupied; }

    const uint8* GetEnd() const { return m_pEnd; }
    uint8* GetEnd() { return m_pEnd; }

protected:
    // return 0 is don't have memory, -1 if it starts from begin or 1 of start from current position
    int32 HasFreeMemory(uint8* pNotFreeBlock, uint64 size, uint32 align);

protected:
    uint8* m_pBegin = nullptr;
    uint8* m_pCurrFree = nullptr;
    uint8* m_pCurrOccupied = nullptr; // must be set from outside
    uint8* m_pEnd = nullptr;
};

} // namespace Cyclone
