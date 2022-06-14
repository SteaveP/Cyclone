#pragma once

#include "CommonVulkan.h"
#include "CommandBufferVulkan.h"

#include "Engine/Render/Types/CommandQueue.h"

namespace Cyclone::Render
{

class RenderBackendVulkan;
class CommandBufferVulkan;

class CommandQueueVulkan : public CCommandQueue
{
public:
    CommandQueueVulkan() = default;
    ~CommandQueueVulkan();

    virtual C_STATUS Init(RenderBackendVulkan* Backend, DeviceHandle Device, CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex);
    virtual void DeInit() override;

    virtual C_STATUS OnBeginRender();
    virtual C_STATUS OnEndRender();

    CommandBufferVulkan* AllocateCommandBuffer();
    void ReturnCommandBuffer(CommandBufferVulkan* commandBuffer);

    virtual C_STATUS Submit(CCommandBuffer** CommandBuffers, uint32_t CommandBuffersCount, bool AutoReturnToPool = true) override;

    C_STATUS SubmitVk(CommandBufferVulkan** CommandBuffers, uint32_t CommandBuffersCount, VkSemaphore WaitSemaphore = VK_NULL_HANDLE, VkFence SubmitCompletedFence = VK_NULL_HANDLE, bool AutoReturnToPool = true);

    VkQueue Get() const { return m_Queue; }
    CommandQueueType GetCommandQueueType() const { return m_QueueType; }
    uint32_t GetQueueFamilyIndex() const { return m_queueFamilyIndex; }
    uint32_t GetQueueIndex() const { return m_queueIndex; }

    RenderBackendVulkan* GetBackendVk() const { return m_BackendVk; }
    DeviceHandle GetDevice() const { return m_Device; }

public:
    VkQueue m_Queue;
    CommandQueueType m_QueueType;

    RenderBackendVulkan* m_BackendVk;
    DeviceHandle m_Device;

    uint32_t m_queueFamilyIndex;
    uint32_t m_queueIndex;

    using CommandBufferVkPtr = UniquePtr<CommandBufferVulkan>;
    Vector<CommandBufferVkPtr> m_freeCommandBuffers;

    struct AllocatedCommandBuffer
    {
        CommandBufferVkPtr CommandBuffer;
        uint32_t Frame;
    };
    Vector<AllocatedCommandBuffer> m_allocatedCommandBuffers;

    // #todo_vk need to track submitted command buffers to wait its semaphores
    Vector<CommandBufferVulkan*> m_submittedCommandBuffers;

};

} // namespace Cyclone::Render
