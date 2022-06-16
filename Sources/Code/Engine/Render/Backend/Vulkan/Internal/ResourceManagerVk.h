#pragma once

#include "CommonVulkan.h"
#include "FrameBufferVk.h"
#include "RenderPassVk.h"
#include "PipelineStateVk.h"

namespace Cyclone::Render
{

class CFrameBufferHandle
{
public:
    Array<void*, 10> AttachmentHandles;
    uint32 AttachmentHandlesCount = 0;
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 Layers = 0;

    static CFrameBufferHandle Create(const CRenderPass& RenderPass);

    bool operator ==(const CFrameBufferHandle& Other) const noexcept
    {
        return AttachmentHandles == Other.AttachmentHandles &&
            AttachmentHandlesCount == Other.AttachmentHandlesCount &&
            Width == Other.Width &&
            Height == Other.Height &&
            Layers == Other.Layers;
    };
};

class RenderPassHasher
{
public:
    std::size_t operator()(const CRenderPass& Key) const noexcept;
};

class CFrameBufferHandleHasher
{
public:
    std::size_t operator()(const CFrameBufferHandle& Key) const noexcept;
};

class CPipelineHandle
{
public:
    PipelineType Type;
    EPipelineFlags Flags; // VkPipelineCreateFlagBits
    PrimitiveTopologyType PrimitiveTopology;
    RasterizerState Rasterizer;
    DepthStencilState DepthStencil;
    BlendState Blend;

    // #todo_vk Multisample state

    const CShader* VertexShader;
    const CShader* PixelShader;
    const CShader* ComputeShader;

    CDeviceHandle DeviceHandle;
    IRendererBackend* RendererBackend = nullptr;

    static CPipelineHandle Create(const CPipelineDesc& Desc);

    bool operator ==(const CPipelineHandle& Other) const noexcept
    {
        return DeviceHandle == Other.DeviceHandle &&
            Type == Other.Type &&
            Flags == Other.Flags &&
            PrimitiveTopology == Other.PrimitiveTopology &&
            Rasterizer == Other.Rasterizer &&
            DepthStencil == Other.DepthStencil &&
            Blend == Other.Blend &&
            VertexShader == Other.VertexShader &&
            PixelShader == Other.PixelShader &&
            ComputeShader == Other.ComputeShader &&
            RendererBackend == Other.RendererBackend;
    }
};

class PipelineHasher
{
public:
    std::size_t operator()(const CPipelineHandle& Key) const noexcept;
};

struct ResourceManagerVkDesc
{
public:
    CDeviceHandle DeviceHandle;
};

// #todo_vk need to support multithreading
class CResourceManagerVk
{
public:
    CResourceManagerVk();
    ~CResourceManagerVk();

    C_STATUS Init(RenderBackendVulkan* RenderBackend, const ResourceManagerVkDesc& Desc);
    void DeInit();

    RenderPassVk* GetRenderPassVk(const CRenderPass& RenderPass);
    FrameBufferVk* GetFrameBufferVk(const CRenderPass& RenderPass);
    PipelineStateVk* GetPipelineStateVk(RenderPassVk* RenderPass, const CPipelineDesc& PipelineDesc);


    RenderBackendVulkan* GetBackendVk() const { return m_BackendVk; }
    VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

protected:
    C_STATUS CreateDescriptorPool();
    void DestroyDescriptorPool();

    RenderPassVk* GetOrCreateRenderPassVk(const CRenderPass& RenderPass);
    FrameBufferVk* GetOrCreateFrameBufferVk(const CRenderPass& RenderPass);
    PipelineStateVk* GetOrCreatePipelineStateVk(RenderPassVk* RenderPass, const CPipelineDesc& PipelineDesc);

protected:
    RenderBackendVulkan* m_BackendVk = nullptr;

    HashMap<CRenderPass, UniquePtr<RenderPassVk>, RenderPassHasher> m_RenderPassCache;
    HashMap<CFrameBufferHandle, UniquePtr<FrameBufferVk>, CFrameBufferHandleHasher> m_FrameBufferCache;
    HashMap<CPipelineHandle, UniquePtr<PipelineStateVk>, PipelineHasher> m_PipelineCache;

    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

    ResourceManagerVkDesc m_Desc;
};

} // namespace Cyclone::Render
