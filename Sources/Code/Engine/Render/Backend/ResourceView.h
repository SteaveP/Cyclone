#pragma once

#include "Engine/Render/CoreRender.h"

namespace Cyclone::Render
{

class ENGINE_API CResourceViewBufferDesc
{
public:
    uint32 Offset = 0;
    uint32 Range = 0;
};

class ENGINE_API CResourceViewTextureDesc
{
public:
    ETextureViewType ViewType = ETextureViewType::Type2D;
    EImageAspectType AspectMask = EImageAspectType::Color;

    uint16 StartMip = 0;
    uint16 MipLevels = uint16(~0u); // VK_REMAINING_MIP_LEVELS
    uint16 StartArrayLayer = 0;
    uint16 ArrayLayers = 1;

    // #todo_vk component swizzling;
};
class ENGINE_API CResourceViewDesc
{
public:
    CHandle<CResource> Resource;
    EResourceFlags Type = EResourceFlags::None;
    EFormatType Format = EFormatType::Undefined;

    CResourceViewBufferDesc Buffer;
    CResourceViewTextureDesc Texture;

    IRendererBackend* Backend = nullptr;

#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif
};

class ENGINE_API CResourceView
{
public:
    DISABLE_COPY_ENABLE_MOVE(CResourceView);

    CResourceView();
    virtual ~CResourceView();

    virtual C_STATUS Init(const CResourceViewDesc& Desc);
    virtual void DeInit();

    CHandle<CResource> GetResource() const { return m_Desc.Resource; }
    const CResourceViewDesc& GetDesc() const { return m_Desc; }

protected:
    CResourceViewDesc m_Desc;
};

} // namespace Cyclone::Render
