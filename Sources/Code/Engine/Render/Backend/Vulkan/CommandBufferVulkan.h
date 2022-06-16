#pragma once

#include "CommonVulkan.h"
#include "Engine/Render/Types/CommandBuffer.h"

namespace Cyclone::Render
{

class CommandQueueVulkan;
class RenderPassVk;
class FrameBufferVk;
class PipelineStateVk;

class CCommandContextVulkan : public CCommandContext
{
    friend CommandQueueVulkan;

public:
    ~CCommandContextVulkan();

    virtual C_STATUS Init(IRenderer* Renderer, CCommandBuffer* CommandBuffer) override;
    virtual void DeInit() override;

    virtual void BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass) override;
    virtual void EndRenderPass(CCommandBuffer* CommandBuffer) override;

    virtual void SetIndexBuffer(CCommandBuffer* CommandBuffer, CBuffer* IndexBuffer, uint32 Offset) override;
    virtual void SetVertexBuffer(CCommandBuffer* CommandBuffer, CBuffer* VertexBuffer, uint32 Slot, uint64 Offset) override;

    virtual Ptr<CPipeline> SetPipeline(CCommandBuffer* CommandBuffer, const CPipelineDesc& Desc) override;

protected:
    RenderBackendVulkan* m_BackendVk = nullptr;
    CommandBufferVulkan* m_CommandBufferVk = nullptr;

    RenderPassVk* m_ActiveRenderPassVk = nullptr;
    FrameBufferVk* m_ActiveFrameBufferVk = nullptr;
    PipelineStateVk* m_ActivePipelineState = nullptr;
};

class CommandBufferVulkan : public CCommandBuffer
{
    friend CommandQueueVulkan;

public:
    ~CommandBufferVulkan();

    C_STATUS Init(CommandQueueVulkan* CommandQueue);
    virtual void DeInit() override;

    virtual C_STATUS Begin() override;
    virtual void End() override;

    virtual void Draw(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, int32 VertexOffset, uint32 FirstInstance) override;

    VkCommandBuffer Get() const { return m_CommandBuffer; }
    VkCommandPool GetCommandPool() const { return m_CommandPool; }
    VkSemaphore GetCompletedSemaphore() const { return m_CompleteSemaphore; }
    CommandQueueVulkan* GetCommandQueue() const { return m_CommandQueue; }

protected:
    void Reset();

protected:
    CommandQueueVulkan* m_CommandQueue = nullptr;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
    VkSemaphore m_CompleteSemaphore = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
