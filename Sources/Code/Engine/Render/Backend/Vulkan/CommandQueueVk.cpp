#include "CommandQueueVk.h"

#include "RenderBackendVk.h"
#include "WindowContextVulkan.h"
#include "CommandBufferVk.h"

namespace Cyclone::Render
{

CommandQueueVk::~CommandQueueVk() = default;

C_STATUS CommandQueueVk::Submit(CommandBufferVk* CommandBuffers, uint32_t CommandBuffersCount, VkSemaphore WaitSemaphore, VkFence SubmitCompletedFence, bool AutoReturnToPool)
{
    CASSERT(CommandBuffersCount <= 10);
    std::array<VkCommandBuffer, 10> RawCommandBuffers;
    std::array<VkSemaphore, 10> RawSignalSemaphores;
    for (uint32_t i = 0; i < CommandBuffersCount; ++i)
    {
        RawCommandBuffers[i] = CommandBuffers[i].Get();
        RawSignalSemaphores[i] = CommandBuffers[i].GetCompletedSemaphore();
        m_submittedCommandBuffers.push_back(&CommandBuffers[i]);
    }

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    if (WaitSemaphore != VK_NULL_HANDLE)
    {
        VkSemaphore WaitSemaphores[] = { WaitSemaphore };
        VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        SubmitInfo.waitSemaphoreCount = 1;
        SubmitInfo.pWaitSemaphores = WaitSemaphores;
        SubmitInfo.pWaitDstStageMask = WaitStages;
    }
    SubmitInfo.commandBufferCount = CommandBuffersCount;
    SubmitInfo.pCommandBuffers = RawCommandBuffers.data();

    SubmitInfo.signalSemaphoreCount = CommandBuffersCount;
    SubmitInfo.pSignalSemaphores = RawSignalSemaphores.data();
    
    if (SubmitCompletedFence != VK_NULL_HANDLE)
    {
        vkResetFences(m_backend->GetWindowContext().GetDevice(), 1, &SubmitCompletedFence);
    }

    VkResult result = vkQueueSubmit(m_queue, 1, &SubmitInfo, SubmitCompletedFence);
    C_ASSERT_VK_SUCCEEDED_RET(result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

Cyclone::Render::CommandBufferVk* CommandQueueVk::AllocateCommandBuffer()
{
    CommandBufferVkPtr CommandBuffer{};

    if (m_freeCommandBuffers.size() != 0)
    {
        CommandBuffer = std::move(m_freeCommandBuffers.back());
        m_freeCommandBuffers.pop_back();
    }
    else
    {
        CommandBuffer = std::make_unique<CommandBufferVk>();
        C_STATUS Result = CommandBuffer->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), nullptr);
    }

    m_allocatedCommandBuffers.emplace_back(AllocatedCommandBuffer{ std::move(CommandBuffer), m_backend->GetCurrentFrame() });

    return m_allocatedCommandBuffers.back().CommandBuffer.get();;
}

void CommandQueueVk::ReturnCommandBuffer(CommandBufferVk* commandBuffer)
{
    // #todo_vk
}

C_STATUS CommandQueueVk::Init(RenderBackendVulkan* Backend, DeviceHandle Device, CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex)
{
    m_backend = Backend;
    m_Device = Device;
    m_queueType = QueueType;
    m_queueFamilyIndex = QueueFamilyIndex;
    m_queueIndex = QueueIndex;

    vkGetDeviceQueue(m_backend->GetGlobalContext().GetLogicalDevice(m_Device).LogicalDeviceHandle, 
        m_queueFamilyIndex, m_queueIndex, &m_queue);

    C_ASSERT_RETURN_VAL(m_queue != VK_NULL_HANDLE, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CommandQueueVk::OnBeginFrame()
{
    auto it = m_allocatedCommandBuffers.begin();
    while (it != m_allocatedCommandBuffers.end())
    {
        if (it->Frame + MAX_FRAMES_IN_FLIGHT <= m_backend->GetCurrentFrame())
        {
            it->CommandBuffer->Reset();

            m_freeCommandBuffers.push_back(std::move(it->CommandBuffer));
            it = m_allocatedCommandBuffers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    m_submittedCommandBuffers.clear();

    return C_STATUS::C_STATUS_OK;
}

} //namespace Cyclone::Render
