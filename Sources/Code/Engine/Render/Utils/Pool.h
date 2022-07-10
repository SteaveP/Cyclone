#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

#include "Engine/Render/Handle.h"

namespace Cyclone::Render
{

template<typename ClassType, typename HandleType>
class CPool
{
public:
    using CHandleType = CHandle<HandleType>;

public:

    CHandleType Create();
    void Destroy(CHandleType Handle);

    ClassType* Get(CHandleType Handle);
    const ClassType* Get(CHandleType Handle) const;

    // ref counting data?
    
    // free list
    // actual data
    struct CMetadata
    {
        uint32 Generation = 0;
        uint32 RefCount = 0;
    };

    // #todo_mem one big aligned allocation
    Vector<CMetadata> m_Metadata;
    Vector<ClassType> m_Data;
    Vector<uint64> m_FreeList;

};

} // namespace Cyclone::Render

#include "Pool.inl"

