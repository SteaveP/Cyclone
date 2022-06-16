#pragma once

#include "Engine/Render/Common.h"

namespace Cyclone::Render
{

class ENGINE_API CTextureAllocation
{
public:
    // #todo_vk
};

class ENGINE_API CTextureDesc
{
public:
    EFormatType Format = EFormatType::Undefined;
    EResourceFlags Flags = EResourceFlags::None;
    EImageType ImageType = EImageType::Type2D;
    ETilingType Tiling = ETilingType::Optimal;
    EImageLayoutType InitialLayout = EImageLayoutType::Undefined;
    EImageUsageType Usage = EImageUsageType::None;
    uint64 Width = 0;
    uint32 Height = 0;
    uint32 Depth = 1;
    uint16 MipLevels = 1;
    uint16 ArrayCount = 1;
    uint16 SamplesCount = 1;

    //imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    RawPtr ExternalBackendResource = nullptr;

    IRendererBackend* Backend = nullptr;
    CDeviceHandle DeviceHandle;

#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif
};

class ENGINE_API CTexture
{
public:
    DISABLE_COPY_ENABLE_MOVE(CTexture);

    CTexture();
    virtual ~CTexture();

    virtual C_STATUS Init(const CTextureDesc& Desc);
    virtual void DeInit();

    virtual void* GetBackendDataPtr() = 0;

    CTextureView GetDefaultView() { return m_DefaultView; }
    const CTextureDesc& GetDesc() const { return m_Desc; }
    const CTextureAllocation& GetAllocation() const { return m_Alloc; }

protected:
    CTextureDesc m_Desc;
    CTextureView m_DefaultView;
    CTextureAllocation m_Alloc;
    // mem ptr? 
    // default views?
    // current state?
};

} // namespace Cyclone::Render
