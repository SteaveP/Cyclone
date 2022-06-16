#pragma once

#include "Engine/Render/Common.h"

namespace Cyclone::Render
{

class ENGINE_API CBufferAllocation
{
public:
    // #todo_vk
};

class ENGINE_API CResourceTransition
{
public:
    // #todo_vk_first
};

class ENGINE_API CBufferDesc
{
public:
    EFormatType Format = EFormatType::Undefined;
    EResourceFlags Flags = EResourceFlags::None;
    EBufferUsageType Usage = EBufferUsageType::None;
    uint64 ByteSize = 0;

    //imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    RawPtr ExternalBackendResource = nullptr;

    IRendererBackend* Backend = nullptr;
    CDeviceHandle Device;

#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif
};

class ENGINE_API CMapData
{
public:
    RawPtr Memory = nullptr;

    operator bool() { return Memory; }
};

class ENGINE_API CBuffer
{
public:
    DISABLE_COPY_ENABLE_MOVE(CBuffer);

    CBuffer();
    virtual ~CBuffer();

    virtual C_STATUS Init(const CBufferDesc& Desc);
    virtual void DeInit();

    virtual CMapData Map() = 0;
    virtual void UnMap(const CMapData& Data) = 0;

    virtual void* GetBackendDataPtr() = 0;

    CTextureView GetDefaultView() { return m_DefaultView; }
    const CBufferDesc& GetDesc() const { return m_Desc; }
    const CBufferAllocation& GetAllocation() const { return m_Alloc; }

protected:
    CBufferDesc m_Desc;
    CTextureView m_DefaultView;
    CBufferAllocation m_Alloc;
    // current state?
};

} // namespace Cyclone::Render
