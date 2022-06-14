#pragma once

#include "CommonVulkan.h"
#include "Engine/Render/Types/CommandBuffer.h"

namespace Cyclone::Render
{

class CommandQueueVulkan;
class RenderPassVk;
class FrameBufferVk;

class CCommandContextVulkan : public CCommandContext
{
    friend CommandQueueVulkan;

public:
    ~CCommandContextVulkan();

    virtual C_STATUS Init(IRenderer* Renderer, CCommandBuffer* CommandBuffer) override;
    virtual void DeInit() override;

    virtual void BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass) override;
    virtual void EndRenderPass(CCommandBuffer* CommandBuffer) override;

protected:
    RenderPassVk* GetOrCreateRenderPass(CommandBufferVulkan* CommandBufferVk, const CRenderPass& RenderPass);
    FrameBufferVk* GetOrCreateFrameBuffer(CommandBufferVulkan* CommandBufferVk, const WindowContextVulkan* WindowContextVk, const CRenderPass& RenderPass);

protected:
    RenderBackendVulkan* m_BackendVk = nullptr;
    CommandBufferVulkan* m_CommandBufferVk = nullptr;

    RenderPassVk* m_ActiveRenderPassVk = nullptr;
    FrameBufferVk* m_ActiveFrameBufferVk = nullptr;

    HashMap<uint64, UniquePtr<RenderPassVk>> m_RenderPassCache;
    HashMap<uint64, UniquePtr<FrameBufferVk>> m_FrameBufferCache;
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
