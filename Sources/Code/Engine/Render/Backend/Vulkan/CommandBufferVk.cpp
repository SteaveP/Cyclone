#include "CommandBufferVk.h"

#include "CommandQueueVk.h"
#include "RenderBackendVk.h"

namespace Cyclone::Render
{

C_STATUS CommandBufferVk::Init(CommandQueueVk* commandQueue)
{
    m_commandQueue = commandQueue;

    WindowContextVulkan& Context = m_commandQueue->GetBackend()->GetWindowContext();

    // create semaphore
    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult Result = vkCreateSemaphore(Context.GetDevice(), &SemaphoreInfo, nullptr, &m_completeSemaphore);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    // create command pool
    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.queueFamilyIndex = m_commandQueue->GetQueueFamilyIndex();
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optional

    Result = vkCreateCommandPool(Context.GetDevice(), &PoolInfo, nullptr, &m_commandPool);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    // create command buffer
    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = m_commandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1;

    Result = vkAllocateCommandBuffers(Context.GetDevice(), &AllocInfo, &m_commandBuffer);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void CommandBufferVk::Begin()
{
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult Result = vkBeginCommandBuffer(m_commandBuffer, &BeginInfo);
    C_ASSERT_VK_SUCCEEDED(Result);
}

void CommandBufferVk::End()
{
    VkResult Result = vkEndCommandBuffer(m_commandBuffer);
    C_ASSERT_VK_SUCCEEDED(Result);
}

void CommandBufferVk::Reset()
{
    VkResult Result = vkResetCommandPool(m_commandQueue->GetBackend()->GetWindowContext().GetDevice(), m_commandPool, 0);
    C_ASSERT_VK_SUCCEEDED(Result);
}

} //namespace Cyclone::Render
