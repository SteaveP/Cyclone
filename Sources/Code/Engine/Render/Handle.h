#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone::Render
{

template<typename T>
class CHandle
{
public:
    ENABLE_COPY(CHandle);
    ENABLE_MOVE(CHandle);

    CHandle() = default;
    CHandle(uint32 Index, uint32 Generation) : m_Index(Index), m_Generation(Generation) {}

    bool IsValid() const { return m_Generation != 0xFFFFFF; }

    bool operator ==(const CHandle<T>& Other) const noexcept = default;

private:
    uint32 m_Index = 0;
    uint32 m_Generation : 24 = 0xFFFFFF;
    uint32 m_DeviceHandle: 8 = 0xFF;

    template<typename ClassType, typename HandleType> friend class CPool;
    friend class CHandleHasher;
    friend struct CDeviceHandle;
};
static_assert(sizeof(CHandle<uint32>) == 8);

// #todo_vk move to other file
class CHandleHasher
{
public:
    template<typename T>
    std::size_t operator()(const CHandle<T>& Key) const noexcept
    {
        std::size_t Value = 0;

        Value ^= std::hash<uint32>()(Key.m_Index);
        Value ^= std::hash<uint32>()(Key.m_Generation) >> 1;

        return Value;
    }
};


} // namespace Cyclone::Render