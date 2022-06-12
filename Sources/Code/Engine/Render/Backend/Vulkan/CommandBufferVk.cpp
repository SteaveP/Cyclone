#include "CommandBufferVk.h"

#include "CommandQueueVk.h"
#include "RenderBackendVk.h"

namespace Cyclone::Render
{

C_STATUS CommandBufferVk::Init(CommandQueueVk* CommandQueue)
{
    m_CommandQueue = CommandQueue;

    VkDevice Device = m_CommandQueue->GetBackend()->GetGlobalContext().GetLogicalDevice(m_CommandQueue->GetDevice()).LogicalDeviceHandle;

    // create semaphore
    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult Result = vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &m_CompleteSemaphore);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    // create command pool
    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.queueFamilyIndex = m_CommandQueue->GetQueueFamilyIndex();
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optional

    Result = vkCreateCommandPool(Device, &PoolInfo, nullptr, &m_CommandPool);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    // create command buffer
    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = m_CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1;

    Result = vkAllocateCommandBuffers(Device, &AllocInfo, &m_CommandBuffer);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void CommandBufferVk::Begin()
{
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult Result = vkBeginCommandBuffer(m_CommandBuffer, &BeginInfo);
    C_ASSERT_VK_SUCCEEDED(Result);
}

void CommandBufferVk::End()
{
    VkResult Result = vkEndCommandBuffer(m_CommandBuffer);
    C_ASSERT_VK_SUCCEEDED(Result);
}

void CommandBufferVk::Reset()
{
    VkDevice Device = m_CommandQueue->GetBackend()->GetGlobalContext().GetLogicalDevice(m_CommandQueue->GetDevice()).LogicalDeviceHandle;

    VkResult Result = vkResetCommandPool(Device, m_CommandPool, 0);
    C_ASSERT_VK_SUCCEEDED(Result);
}

} //namespace Cyclone::Render
