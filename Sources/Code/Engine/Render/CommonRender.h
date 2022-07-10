#pragma once

#include "CoreRender.h"

namespace Cyclone::Render
{

//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////
union CClearColor
{
    Vec4 Color;
    Vec2 DepthStencil;

    CClearColor() : CClearColor(Vec4{ 0,0,0,0 }) {};
    CClearColor(Vec4 ColorArg) : Color(ColorArg) {}
    CClearColor(Vec2 DepthStencilArg) : DepthStencil(DepthStencilArg) {}

    bool operator==(const CClearColor& Other) const noexcept { return Color == Other.Color; }
};

class ENGINE_API CRenderTarget
{
public:
    CHandle<CResource> Texture;

    // #todo_vk Additional views for resource
    CHandle<CResourceView> RenderTargetView;
    CHandle<CResourceView> DepthStencilView;
    CHandle<CResourceView> ShaderResourceView;
    CHandle<CResourceView> UnorderedAccessView;

    bool IsValid() const { return Texture.IsValid(); }
    bool operator ==(const CRenderTarget& Other) const noexcept = default;
};

class CRenderTargetSlot
{
public:
    CRenderTarget RenderTarget;

    // To this state resource will be transitioned after render pass would be finished
    EImageLayoutType FinalLayout = EImageLayoutType::Undefined;
    EImageUsageType FinalUsage = EImageUsageType::None;

    ERenderTargetLoadOp LoadOp = ERenderTargetLoadOp::Load;
    ERenderTargetStoreOp StoreOp = ERenderTargetStoreOp::Store;
    ERenderTargetLoadOp StencilLoadOp = ERenderTargetLoadOp::Load;
    ERenderTargetStoreOp StencilStoreOp = ERenderTargetStoreOp::Store;

    CClearColor ClearValue;

    bool IsValid() const { return RenderTarget.Texture.IsValid(); }

    bool operator ==(const CRenderTargetSlot& Other) const noexcept = default;
};

class CRenderTargetSet
{
public:
    static const uint32 MaxRenderTargets = 10;

    uint32 RenderTargetsCount = 0;
    Array<CRenderTargetSlot, MaxRenderTargets> RenderTargets;

    CRenderTargetSlot DepthScentil;

    bool operator ==(const CRenderTargetSet& Other) const noexcept = default;
};

class CRenderPass
{
public:
    CRenderTargetSet RenderTargetSet;
    Vec4 ViewportExtent;

    bool operator ==(const CRenderPass& Other) const noexcept = default;
};

class CSamplerDesc
{
public:
    // #todo_vk_sampler fixme
    uint32 dummy;

    IRendererBackend* Backend = nullptr;
    CDeviceHandle DeviceHandle;

    bool operator ==(const CSamplerDesc& Other) const noexcept = default;
};

class CSampler
{
public:
    DISABLE_COPY_ENABLE_MOVE(CSampler);
    CSampler() = default;
    virtual ~CSampler() = default;

    virtual C_STATUS Init(const CSamplerDesc& Desc) { m_Desc = Desc; return C_STATUS::C_STATUS_OK; }
    virtual void DeInit() {}

    const CSamplerDesc& GetDesc() const { return m_Desc; }

protected:
    CSamplerDesc m_Desc;
};

struct CDescriptorBinding
{
    uint32 BindingIndex = 0;
    EDescriptorType DescriptorType = EDescriptorType::Buffer;
    uint32 DescriptorCount = 1; // > 1 Mean arrays
    uint32 StageFlags = 0xFFFFFFFF; // #todo_vk // means all stages
    EDescriptorSetLayoutFlags Flags = EDescriptorSetLayoutFlags::None;
    Vector<CSamplerDesc> ImmutableSamplers;

    bool operator ==(const CDescriptorBinding& Other) const noexcept = default;
};

struct CDescriptorSetLayout
{
    EDescriptorSetLayoutFlags Flags = EDescriptorSetLayoutFlags::None;
    Vector<CDescriptorBinding> Bindings;

    bool operator ==(const CDescriptorSetLayout& Other) const noexcept = default;
};

struct CShaderBindingSet
{
    Vector<CDescriptorSetLayout> SetLayouts;

    bool operator ==(const CShaderBindingSet& Other) const noexcept = default;
};

struct CDescriptorSet
{
    // #todo_vk_material
    void* RawData = nullptr;
};

struct CTextureSubresourceRange
{
    static const uint32 RemainingMipLevels = (~0U);
    static const uint32 RemainingArrayLayers = (~0U);

    EImageAspectType AspectMask = EImageAspectType::Color;
    uint32 BaseMipLevel = 0;
    uint32 LevelCount = RemainingMipLevels;
    uint32 BaseArrayLayer = 0;
    uint32 LayerCount = RemainingArrayLayers;
};

} // namespace Cyclone::Render
