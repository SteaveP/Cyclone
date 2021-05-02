#pragma once

#include "Common/CommonVulkan.h"

namespace Cyclone::Render
{

class CommandQueueVk;

class CommandBufferVk
{
public:
    C_STATUS Init(CommandQueueVk* commandQueue);

    void Begin();
    void End();

    void Reset();

    VkCommandBuffer Get() const { return m_commandBuffer; }
    VkCommandPool GetCommandPool() const { return m_commandPool; }
    VkSemaphore GetCompletedSemaphore() const { return m_completeSemaphore; }
protected:
    CommandQueueVk* m_commandQueue;

    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    VkSemaphore m_completeSemaphore;
};

} // namespace Cyclone::Render
