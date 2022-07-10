#include "CommandQueueVulkan.h"

#include "RenderBackendVulkan.h"
#include "CommandBufferVulkan.h"

namespace Cyclone::Render
{

CCommandQueueVulkan::CCommandQueueVulkan() = default;
CCommandQueueVulkan::CCommandQueueVulkan(CCommandQueueVulkan&& Other) noexcept : CCommandQueue(MoveTemp(Other))
{
    std::swap(m_Queue, Other.m_Queue);
    std::swap(m_QueueType, Other.m_QueueType);
    std::swap(m_BackendVk, Other.m_BackendVk);
    std::swap(m_DeviceHandle, Other.m_DeviceHandle);
    std::swap(m_QueueFamilyIndex, Other.m_QueueFamilyIndex);
    std::swap(m_QueueIndex, Other.m_QueueIndex);
    std::swap(m_FreeCommandBuffers, Other.m_FreeCommandBuffers);
    std::swap(m_AllocatedCommandBuffers, Other.m_AllocatedCommandBuffers);
    std::swap(m_SubmittedCommandBuffers, Other.m_SubmittedCommandBuffers);
}
CCommandQueueVulkan& CCommandQueueVulkan::operator=(CCommandQueueVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CCommandQueue::operator=(MoveTemp(Other));
        std::swap(m_Queue, Other.m_Queue);
        std::swap(m_QueueType, Other.m_QueueType);
        std::swap(m_BackendVk, Other.m_BackendVk);
        std::swap(m_DeviceHandle, Other.m_DeviceHandle);
        std::swap(m_QueueFamilyIndex, Other.m_QueueFamilyIndex);
        std::swap(m_QueueIndex, Other.m_QueueIndex);
        std::swap(m_FreeCommandBuffers, Other.m_FreeCommandBuffers);
        std::swap(m_AllocatedCommandBuffers, Other.m_AllocatedCommandBuffers);
        std::swap(m_SubmittedCommandBuffers, Other.m_SubmittedCommandBuffers);
    }
    return *this;
}
CCommandQueueVulkan::~CCommandQueueVulkan()
{
    DeInitImpl();

    CASSERT(m_AllocatedCommandBuffers.empty() && m_FreeCommandBuffers.empty());
}

C_STATUS CCommandQueueVulkan::SubmitVk(CCommandBufferVulkan** CommandBuffers, uint32_t CommandBuffersCount, VkSemaphore WaitSemaphore, VkFence SubmitCompletedFence, bool AutoReturnToPool)
{
    CASSERT(CommandBuffersCount <= 10);
    Array<VkCommandBufferSubmitInfo, 10> RawCommandBuffers{};
    Array<VkSemaphoreSubmitInfo, 10> WaitSemaphores{};
    Array<VkSemaphoreSubmitInfo, 10> SignalSemaphores{};

    for (uint32_t i = 0; i < CommandBuffersCount; ++i)
    {
        auto& SignalSemaphore = SignalSemaphores[i];
        SignalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        SignalSemaphore.semaphore = CommandBuffers[i]->GetCompletedSemaphore();
        SignalSemaphore.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // #todo_vk pass stage mask

        auto& CommandBuffer = RawCommandBuffers[i];
        CommandBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        CommandBuffer.commandBuffer = CommandBuffers[i]->Get();
        m_SubmittedCommandBuffers.push_back(CommandBuffers[i]);
    }

    VkSubmitInfo2 SubmitInfo{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };

    if (WaitSemaphore != VK_NULL_HANDLE)
    {
        auto& WaitSemaphoreInfo =WaitSemaphores[0];
        WaitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        WaitSemaphoreInfo.semaphore = WaitSemaphore;        
        WaitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; // #todo_vk pass stage mask

        SubmitInfo.waitSemaphoreInfoCount = 1;
        SubmitInfo.pWaitSemaphoreInfos = WaitSemaphores.data();
    }
    SubmitInfo.commandBufferInfoCount = CommandBuffersCount;
    SubmitInfo.pCommandBufferInfos = CommandBuffersCount == 0 ? nullptr : RawCommandBuffers.data();

    SubmitInfo.signalSemaphoreInfoCount = CommandBuffersCount;
    SubmitInfo.pSignalSemaphoreInfos = CommandBuffersCount == 0 ? nullptr : SignalSemaphores.data();
    
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_DeviceHandle);

    if (SubmitCompletedFence != VK_NULL_HANDLE)
    {
        VK_CALL(Device, vkResetFences(Device.DeviceVk, 1, &SubmitCompletedFence));
    }

    VkResult ResultVk = VK_CALL(Device, vkQueueSubmit2(m_Queue, 1, &SubmitInfo, SubmitCompletedFence));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

    if (AutoReturnToPool)
    {
        for (uint32_t i = 0; i < CommandBuffersCount; ++i)
        {
            ReturnCommandBuffer(CommandBuffers[i]);
        }
    }

    return C_STATUS::C_STATUS_OK;
}

CCommandBufferVulkan* CCommandQueueVulkan::AllocateCommandBufferVk()
{
    CommandBufferVkPtr CommandBuffer{};

    if (m_FreeCommandBuffers.size() != 0)
    {
        CommandBuffer = MoveTemp(m_FreeCommandBuffers.back());
        m_FreeCommandBuffers.pop_back();
    }
    else
    {
        CommandBuffer = MakeUnique<CCommandBufferVulkan>();
        C_STATUS Result = CommandBuffer->Init(this);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), nullptr);
    }

    m_AllocatedCommandBuffers.emplace_back(CAllocatedCommandBuffer{ MoveTemp(CommandBuffer), m_BackendVk->GetRenderer()->GetCurrentFrame() });

    return m_AllocatedCommandBuffers.back().CommandBuffer.get();;
}

void CCommandQueueVulkan::ReturnCommandBufferVk(CCommandBufferVulkan* CommandBuffer)
{
    // #todo_vk
    //CASSERT(false);
}

C_STATUS CCommandQueueVulkan::Init(CRenderBackendVulkan* Backend, CDeviceHandle DeviceHandle, CommandQueueType QueueType, uint32_t QueueFamilyIndex, uint32_t QueueIndex)
{
    // #todo_vk call base method
    m_Backend = Backend;
    m_BackendVk = Backend;
    m_DeviceHandle = DeviceHandle;
    m_QueueType = QueueType;
    m_QueueFamilyIndex = QueueFamilyIndex;
    m_QueueIndex = QueueIndex;

    auto& Device= m_BackendVk->GetDeviceManager().GetDevice(m_DeviceHandle);
    VK_CALL(Device, vkGetDeviceQueue(Device.DeviceVk,
        m_QueueFamilyIndex, m_QueueIndex, &m_Queue));

    C_ASSERT_RETURN_VAL(m_Queue != VK_NULL_HANDLE, C_STATUS::C_STATUS_ERROR);

#if ENABLE_DEBUG_RENDER_BACKEND
    SetDebugNameVk("Queue Type" + ToString((uint32)QueueType) + " Device" + ToString((uint32)DeviceHandle.PhysDeviceHandle) + ToString((uint32)DeviceHandle.DeviceHandle),
        VK_OBJECT_TYPE_QUEUE, (uint64)m_Queue, Device);
#endif

    return C_STATUS::C_STATUS_OK;
}

void CCommandQueueVulkan::DeInit()
{
    DeInitImpl();
    CCommandQueue::DeInit();
}

void CCommandQueueVulkan::DeInitImpl() noexcept
{
    m_AllocatedCommandBuffers.clear();
    m_FreeCommandBuffers.clear();
    m_SubmittedCommandBuffers.clear();

    m_Queue = nullptr;
}

C_STATUS CCommandQueueVulkan::OnBeginRender()
{
    C_STATUS Result = CCommandQueue::OnBeginRender();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    auto it = m_AllocatedCommandBuffers.begin();
    while (it != m_AllocatedCommandBuffers.end())
    {
        if (it->Frame + m_BackendVk->GetRenderer()->GetFramesInFlightCount() <= m_BackendVk->GetRenderer()->GetCurrentFrame())
        {
            it->CommandBuffer->Reset();

            m_FreeCommandBuffers.push_back(MoveTemp(it->CommandBuffer));
            it = m_AllocatedCommandBuffers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    m_SubmittedCommandBuffers.clear();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CCommandQueueVulkan::OnEndRender()
{
    C_STATUS Result = CCommandQueue::OnEndRender();
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CCommandQueueVulkan::Submit(CCommandBuffer** CommandBuffers, uint32_t CommandBuffersCount, bool AutoReturnToPool)
{
    C_STATUS Result = CCommandQueue::Submit(CommandBuffers, CommandBuffersCount, AutoReturnToPool);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return SubmitVk(reinterpret_cast<CCommandBufferVulkan**>(CommandBuffers), CommandBuffersCount, nullptr, nullptr, AutoReturnToPool);
}

CCommandBuffer* CCommandQueueVulkan::AllocateCommandBuffer()
{
    return AllocateCommandBufferVk();
}

void CCommandQueueVulkan::ReturnCommandBuffer(CCommandBuffer* CommandBuffer)
{
    ReturnCommandBufferVk(BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan));
}

} //namespace Cyclone::Render
