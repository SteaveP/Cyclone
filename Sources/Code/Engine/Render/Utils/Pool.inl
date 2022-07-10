#include "Pool.h"

namespace Cyclone::Render
{

template<typename ClassType, typename HandleType>
inline CPool<ClassType, HandleType>::CHandleType CPool<ClassType, HandleType>::Create()
{
    CHandleType Handle {static_cast<uint32>(m_Data.size()), 0u};

    // #todo_vk_resource implement free element tracking and resource deletion + ref counting?
    m_Data.emplace_back();
    m_Metadata.emplace_back();

    return Handle;
}

template<typename ClassType, typename HandleType>
void CPool<ClassType, HandleType>::Destroy(CHandleType Handle)
{
    CASSERT(Handle.m_Index <= m_Data.size());
    
    auto& Metadata = m_Metadata[Handle.m_Index];
    CASSERT(Handle.m_Generation == Metadata.Generation);

    ++Metadata.Generation;

    m_Data[Handle.m_Index].DeInit();
}

template<typename ClassType, typename HandleType>
const ClassType* Cyclone::Render::CPool<ClassType, HandleType>::Get(CHandleType Handle) const
{
    CASSERT(Handle.m_Index <= m_Data.size());
    CASSERT(Handle.m_Generation == m_Metadata[Handle.m_Index].Generation);
    return &m_Data[Handle.m_Index];
}

template<typename ClassType, typename HandleType>
ClassType* Cyclone::Render::CPool<ClassType, HandleType>::Get(CHandleType Handle)
{
    CASSERT(Handle.m_Index <= m_Data.size());
    CASSERT(Handle.m_Generation == m_Metadata[Handle.m_Index].Generation);
    return &m_Data[Handle.m_Index];
}

} // namespace Cyclone::Render
