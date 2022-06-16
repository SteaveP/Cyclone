#include "CommandQueueVulkan.h"

#include "RenderBackendVulkan.h"
#include "WindowContextVulkan.h"
#include "CommandBufferVulkan.h"

namespace Cyclone::Render
{

CommandQueueVulkan::~CommandQueueVulkan()
{
    DeInit();

    CASSERT(m_allocatedCommandBuffers.empty() && m_freeCommandBuffers.empty());
}

C_STATUS CommandQueueVulkan::SubmitVk(CommandBufferVulkan** CommandBuffers, uint32_t CommandBuffersCount, VkSemaphore WaitSemaphore, VkFence SubmitCompletedFence, bool AutoReturnToPool)
{
    CASSERT(CommandBuffersCount <= 10);
    Array<VkCommandBuffer, 10> RawCommandBuffers;
    Array<VkSemaphore, 10> RawSignalSemaphores;
    for (uint32_t i = 0; i < CommandBuffersCount; ++i)
    {
        RawCommandBuffers[i] = CommandBuffers[i]->Get();
        RawSignalSemaphores[i] = CommandBuffers[i]->GetCompletedSemaphore();
        m_submittedCommandBuffers.push_back(CommandBuffers[i]);
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
        vkResetFences(m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Device).LogicalDeviceHandle, 1, &SubmitCompletedFence);
    }

    VkResult result = vkQueueSubmit(m_Queue, 1, &SubmitInfo, SubmitCompletedFence);
    C_ASSERT_VK_SUCCEEDED_RET(result, C_STATUS::C_STATUS_ERROR);

    if (AutoReturnToPool)
    {
        for (uint32_t i = 0; i < CommandBuffersCount; ++i)
        {
            ReturnCommandBuffer(CommandBuffers[i]);
        }
    }

    return C_STATUS::C_STATUS_OK;
}

Cyclone::Render::CommandBufferVulkan* CommandQueueVulkan::AllocateCommandBuffer()
{
    CommandBufferVkPtr CommandBuffer{};

    if (m_freeCommandBuffers.size() != 0)
    {
        CommandBuffer = MoveTemp(m_freeCommandBuffers.back());
        m_freeCommandBuffers.pop_back();
    }
    else
    {
        CommandBuffer = MakeUnique<CommandBufferVulkan>();
        C_STATUS Result = CommandBuffer->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), nullptr);
    }

    m_allocatedCommandBuffers.emplace_back(AllocatedCommandBuffer{ MoveTemp(CommandBuffer), m_BackendVk->GetRenderer()->GetCurrentFrame() });

    return m_allocatedCommandBuffers.back().CommandBuffer.get();;
}

void CommandQueueVulkan::ReturnCommandBuffer(CommandBufferVulkan* commandBuffer)
{
    // #todo_vk
    //CASSERT(false);
}

C_STATUS CommandQueueVulkan::Init(RenderBackendVulkan* Backend, CDeviceHandle Device, CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex)
{
    // #todo_vk call base method
    m_Backend = Backend;
    m_BackendVk = Backend;
    m_Device = Device;
    m_QueueType = QueueType;
    m_queueFamilyIndex = QueueFamilyIndex;
    m_queueIndex = QueueIndex;

    vkGetDeviceQueue(m_BackendVk->GetGlobalContext().GetLogicalDevice(m_Device).LogicalDeviceHandle, 
        m_queueFamilyIndex, m_queueIndex, &m_Queue);

    C_ASSERT_RETURN_VAL(m_Queue != VK_NULL_HANDLE, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void CommandQueueVulkan::DeInit()
{
    m_allocatedCommandBuffers.clear();
    m_freeCommandBuffers.clear();
    m_submittedCommandBuffers.clear();

    m_Queue = nullptr;

    CCommandQueue::DeInit();
}

C_STATUS CommandQueueVulkan::OnBeginRender()
{
    C_STATUS Result = CCommandQueue::OnBeginRender();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    auto it = m_allocatedCommandBuffers.begin();
    while (it != m_allocatedCommandBuffers.end())
    {
        if (it->Frame + MAX_FRAMES_IN_FLIGHT <= m_BackendVk->GetRenderer()->GetCurrentFrame())
        {
            it->CommandBuffer->Reset();

            m_freeCommandBuffers.push_back(MoveTemp(it->CommandBuffer));
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

C_STATUS CommandQueueVulkan::OnEndRender()
{
    C_STATUS Result = CCommandQueue::OnEndRender();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CommandQueueVulkan::Submit(CCommandBuffer** CommandBuffers, uint32_t CommandBuffersCount, bool AutoReturnToPool)
{
    C_STATUS Result = CCommandQueue::Submit(CommandBuffers, CommandBuffersCount, AutoReturnToPool);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return SubmitVk(reinterpret_cast<CommandBufferVulkan**>(CommandBuffers), CommandBuffersCount, nullptr, nullptr, AutoReturnToPool);
}

} //namespace Cyclone::Render
