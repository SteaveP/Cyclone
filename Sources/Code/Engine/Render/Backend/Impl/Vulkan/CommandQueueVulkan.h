#pragma once

#include "CommonVulkan.h"

#include "Engine/Render/Backend/CommandQueue.h"

namespace Cyclone::Render
{

class CCommandQueueVulkan : public CCommandQueue
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CCommandQueueVulkan);

    CCommandQueueVulkan();
    ~CCommandQueueVulkan();

    virtual C_STATUS Init(CRenderBackendVulkan* Backend, CDeviceHandle DeviceHandle, CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex);
    virtual void DeInit() override;

    virtual C_STATUS OnBeginRender();
    virtual C_STATUS OnEndRender();

    virtual CCommandBuffer* AllocateCommandBuffer() override;
    virtual void ReturnCommandBuffer(CCommandBuffer* CommandBuffer) override;

    CCommandBufferVulkan* AllocateCommandBufferVk();
    void ReturnCommandBufferVk(CCommandBufferVulkan* CommandBuffer);

    virtual CommandQueueType GetCommandQueueType() const override { return m_QueueType; }

    virtual C_STATUS Submit(CCommandBuffer** CommandBuffers, uint32_t CommandBuffersCount, bool AutoReturnToPool = true) override;

    C_STATUS SubmitVk(CCommandBufferVulkan** CommandBuffers, uint32_t CommandBuffersCount, VkSemaphore WaitSemaphore = VK_NULL_HANDLE, VkFence SubmitCompletedFence = VK_NULL_HANDLE, bool AutoReturnToPool = true);

    VkQueue Get() const { return m_Queue; }
    uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }
    uint32_t GetQueueIndex() const { return m_QueueIndex; }

    CRenderBackendVulkan* GetBackendVk(){ return m_BackendVk; }
    const CRenderBackendVulkan* GetBackendVk() const { return m_BackendVk; }
    
    uint32 GetSubmittedCommandBufferCount() const { return static_cast<uint32>(m_SubmittedCommandBuffers.size()); }
    CCommandBufferVulkan* GetSubmittedCommandBuffer(uint32 Index) const { return m_SubmittedCommandBuffers[Index]; }

private:
    void DeInitImpl() noexcept;

protected:
    VkQueue m_Queue = VK_NULL_HANDLE;
    CommandQueueType m_QueueType = CommandQueueType::Graphics;

    CRenderBackendVulkan* m_BackendVk = nullptr;

    uint32_t m_QueueFamilyIndex = 0;
    uint32_t m_QueueIndex = 0;

    using CommandBufferVkPtr = UniquePtr<CCommandBufferVulkan>;
    Vector<CommandBufferVkPtr> m_FreeCommandBuffers;

    struct CAllocatedCommandBuffer
    {
        CommandBufferVkPtr CommandBuffer;
        uint32_t Frame;
    };
    Vector<CAllocatedCommandBuffer> m_AllocatedCommandBuffers;

    // #todo_vk need to track submitted command buffers to wait its semaphores
    Vector<CCommandBufferVulkan*> m_SubmittedCommandBuffers;

};

} // namespace Cyclone::Render
