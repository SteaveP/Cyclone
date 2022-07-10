#pragma once

#include "Engine/Render/CoreRender.h"

namespace Cyclone::Render
{

class ENGINE_API CMapData
{
public:
    RawPtr Memory = nullptr;

    operator bool() { return Memory; }
};

class ENGINE_API CResourceAllocation
{
public:
    // #todo_vk
};

class ENGINE_API CResourceBufferDesc
{
public:
    EBufferUsageType Usage = EBufferUsageType::None;
    uint64 ByteSize = 0;
};

class ENGINE_API CResourceTextureDesc
{
public:
    EImageUsageType Usage = EImageUsageType::None;
    EImageUsageType InitialUsage = EImageUsageType::None; // todo_vk_material EResourceUsageType

    ETextureType ImageType = ETextureType::Type2D;
    ETilingType Tiling = ETilingType::Optimal;

    uint32 Width = 0;
    uint32 Height = 0;
    uint32 Depth = 1;
    uint16 MipLevels = 1;
    uint16 ArrayCount = 1;
    uint16 SamplesCount = 1;
};

class ENGINE_API CResourceDesc
{
public:
    EFormatType Format = EFormatType::Undefined;
    EResourceFlags Flags = EResourceFlags::None;
    EImageLayoutType InitialLayout = EImageLayoutType::Undefined;
    //imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CResourceBufferDesc Buffer;
    CResourceTextureDesc Texture;

    RawPtr ExternalBackendResource = nullptr;

    IRendererBackend* Backend = nullptr;
    CDeviceHandle DeviceHandle;

#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif
};

class ENGINE_API CResource
{
public:
    DISABLE_COPY_ENABLE_MOVE(CResource);

    CResource();
    virtual ~CResource();

    virtual C_STATUS Init(const CResourceDesc& Desc);
    virtual void DeInit();

    virtual RawPtr GetBackendDataPtr() = 0;

    virtual CMapData Map() = 0;
    virtual void UnMap(const CMapData& Data) = 0;

    const CResourceDesc& GetDesc() const { return m_Desc; }
    const CResourceAllocation& GetAllocation() const { return m_Alloc; }

    EImageLayoutType GetTraceableLayout() const { return m_TraceableLayout; }
    void UpdateTraceableLayout(EImageLayoutType Layout) { m_TraceableLayout = Layout; } // do this without checks

    EImageUsageType GetTraceableUsageType() const { return m_TraceableTextureUsageType; } 
    void UpdateTraceableUsageType(EImageUsageType UsageType) { m_TraceableTextureUsageType = UsageType; } // do this without checks

private:
    void DeInitImpl() noexcept;

protected:

    CResourceDesc m_Desc;
    CResourceAllocation m_Alloc;

    // #todo_vk default view?

    // Traceable layout in CPU timeline, includes pending state transitions that not submitted to GPU yet
    EImageLayoutType m_TraceableLayout = EImageLayoutType::Undefined;
    EImageUsageType m_TraceableTextureUsageType = EImageUsageType::None;
};

} // namespace Cyclone::Render
