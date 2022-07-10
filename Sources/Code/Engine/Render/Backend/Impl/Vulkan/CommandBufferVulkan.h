#pragma once

#include "CommonVulkan.h"

#include "Engine/Render/Backend/CommandBuffer.h"

#include "Internal/PipelineStateVulkan.h"

namespace Cyclone::Render
{

class CPipelineStateVulkan;

class CCommandContextVulkan : public CCommandContext
{
    friend CCommandQueueVulkan;

public:
    DISABLE_COPY_ENABLE_MOVE(CCommandContextVulkan);

    CCommandContextVulkan();
    ~CCommandContextVulkan();

    virtual C_STATUS Init(IRendererBackend* Backend, CCommandBuffer* CommandBuffer) override;
    virtual void DeInit() override;

    virtual void BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass) override;
    virtual void EndRenderPass(CCommandBuffer* CommandBuffer) override;

    virtual void OnBegin(CCommandBuffer* CommandBuffer) override;
    virtual void OnEnd(CCommandBuffer* CommandBuffer) override;

    virtual void SetIndexBuffer(CCommandBuffer* CommandBuffer, CHandle<CResource> IndexBuffer, uint64 Offset, EFormatType Format = EFormatType::R16_UINT) override;
    virtual void SetVertexBuffer(CCommandBuffer* CommandBuffer, CHandle<CResource> VertexBuffer, uint32 Slot, uint64 Offset) override;

    virtual void InsertPipelineBarrier(CCommandBuffer* CommandBuffer, const CPipelineBarrier& Barrier, bool ForceFlush) override;
    virtual void FLushPendingPipelineBarriers(CCommandBuffer* CommandBuffer) override;

    virtual void SetPipeline(CCommandBuffer* CommandBuffer, CHandle<CPipelineState> Pipeline) override;

    CHandle<CPipelineState> GetCurrentPipeline() const { return m_ActivePipelineState; }

private:
    void DeInitImpl();

protected:
    CRenderBackendVulkan* m_BackendVk = nullptr;
    CCommandBufferVulkan* m_CommandBufferVk = nullptr;

    CHandle<CPipelineState> m_ActivePipelineState;
};

class CCommandBufferVulkan : public CCommandBuffer
{
    friend CCommandQueueVulkan;

public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CCommandBufferVulkan);

    CCommandBufferVulkan();
    ~CCommandBufferVulkan();

    C_STATUS Init(CCommandQueueVulkan* CommandQueue);
    virtual void DeInit() override;

    virtual C_STATUS Begin() override;
    virtual void End() override;

    virtual void Draw(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, int32 VertexOffset, uint32 FirstInstance) override;

    virtual void CopyBuffer(const CCopyBufferDesc& Desc) override;
    virtual void CopyBufferToTexture(const CCopyBufferToTextureDesc& Desc) override;

    VkCommandBuffer Get() const { return m_CommandBuffer; }
    VkCommandPool GetCommandPool() const { return m_CommandPool; }
    VkSemaphore GetCompletedSemaphore() const { return m_CompleteSemaphore; }
    CCommandQueueVulkan* GetCommandQueueVk() const { return m_CommandQueueVk; }

    CRenderBackendVulkan* GetBackendVk() { return m_BackendVk; }
    const CRenderBackendVulkan* GetBackendVk() const { return m_BackendVk; }

private:
    void DeInitImpl();
    void Reset();

protected:
    CCommandQueueVulkan* m_CommandQueueVk = nullptr;
    CRenderBackendVulkan* m_BackendVk = nullptr;
    CCommandContextVulkan* m_ContextVk = nullptr;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
    VkSemaphore m_CompleteSemaphore = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
