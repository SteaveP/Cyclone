#pragma once

#include "Common/CommonVulkan.h"

namespace Cyclone::Render
{

class RenderBackendVulkan;
class CommandBufferVk;

class CommandQueueVk
{
public:
    CommandQueueVk() = default;
    ~CommandQueueVk();
    C_STATUS Init(RenderBackendVulkan* Backend, DeviceHandle Device, CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex);

    C_STATUS OnBeginFrame();

    CommandBufferVk* GetImmediateCommandBuffer();
    CommandBufferVk* AllocateCommandBuffer();
    void ReturnCommandBuffer(CommandBufferVk* commandBuffer);

    C_STATUS Submit(CommandBufferVk* CommandBuffers, uint32_t CommandBuffersCount, VkSemaphore WaitSemaphore = VK_NULL_HANDLE, VkFence SubmitCompletedFence = VK_NULL_HANDLE, bool AutoReturnToPool = true);

    VkQueue Get() const { return m_queue; }
    CommandQueueType GetCommandQueueType() const { return m_queueType; }
    uint32_t GetQueueFamilyIndex() const { return m_queueFamilyIndex; }
    uint32_t GetQueueIndex() const { return m_queueIndex; }

    RenderBackendVulkan* GetBackend() const { return m_backend; }

public:

    VkQueue m_queue;
    CommandQueueType m_queueType;

    DeviceHandle m_Device;

    using CommandBufferVkPtr = std::unique_ptr<CommandBufferVk>;
    std::vector<CommandBufferVkPtr> m_freeCommandBuffers;

    struct AllocatedCommandBuffer
    {
        CommandBufferVkPtr CommandBuffer;
        uint32_t Frame;
    };
    std::vector<AllocatedCommandBuffer> m_allocatedCommandBuffers;
    uint32_t m_queueFamilyIndex;
    uint32_t m_queueIndex;

    // #todo_vk need to track submitted command buffers to wait its semaphores
    std::vector<CommandBufferVk*> m_submittedCommandBuffers;

    RenderBackendVulkan* m_backend;
};

} // namespace Cyclone::Render
