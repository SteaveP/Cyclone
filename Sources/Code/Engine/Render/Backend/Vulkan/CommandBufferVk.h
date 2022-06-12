#pragma once

#include "Common/CommonVulkan.h"

namespace Cyclone::Render
{

class CommandQueueVk;

class CommandBufferVk
{
public:
    C_STATUS Init(CommandQueueVk* CommandQueue);

    void Begin();
    void End();

    void Reset();

    VkCommandBuffer Get() const { return m_CommandBuffer; }
    VkCommandPool GetCommandPool() const { return m_CommandPool; }
    VkSemaphore GetCompletedSemaphore() const { return m_CompleteSemaphore; }
protected:
    CommandQueueVk* m_CommandQueue;

    VkCommandPool m_CommandPool;
    VkCommandBuffer m_CommandBuffer;
    VkSemaphore m_CompleteSemaphore;
};

} // namespace Cyclone::Render
