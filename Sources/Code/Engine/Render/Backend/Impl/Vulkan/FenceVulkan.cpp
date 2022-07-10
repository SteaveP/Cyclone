#include "FenceVulkan.h"

#include "RenderBackendVulkan.h"
#include "CommandQueueVulkan.h"
#include "Internal/DeviceManagerVulkan.h"

namespace Cyclone::Render
{

CFenceVulkan::CFenceVulkan() = default;
CFenceVulkan::CFenceVulkan(CFenceVulkan&& Other) noexcept : CFence(MoveTemp(Other))
{
    std::swap(this->m_Semaphore, Other.m_Semaphore);
}

CFenceVulkan& CFenceVulkan::operator =(CFenceVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CFence::operator=(MoveTemp(Other));
        std::swap(this->m_Semaphore, Other.m_Semaphore);
    }
    return *this;
}

CFenceVulkan::~CFenceVulkan()
{
    DeInitImpl();

    CASSERT(m_Semaphore == VK_NULL_HANDLE);
}

C_STATUS CFenceVulkan::Init(const CFenceDesc& Desc)
{
    C_STATUS Result = CFence::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
    const auto& Device = BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

    VkSemaphoreTypeCreateInfo TypeInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
    TypeInfo.initialValue = Desc.InitialValue;
    TypeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo Info{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    Info.pNext = &TypeInfo;

    VkResult ResultVk = VK_CALL(Device, vkCreateSemaphore(Device.DeviceVk, &Info, nullptr, &m_Semaphore));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_INVALID_ARG);

#if ENABLE_DEBUG_RENDER_BACKEND
    if (m_Desc.Name.empty() == false)
    {
        SetDebugNameVk(m_Desc.Name, VK_OBJECT_TYPE_SEMAPHORE, (uint64)m_Semaphore, Device);
    }
#endif

    return C_STATUS::C_STATUS_OK;
}

void CFenceVulkan::DeInit()
{
    DeInitImpl();
    CFence::DeInit();
}

void CFenceVulkan::DeInitImpl()
{
    if (m_Semaphore)
    {
        CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);

        BackendVk->GetDisposalManagerVk(m_Desc.DeviceHandle)->AddDisposable([Backend = BackendVk, DeviceHandle = m_Desc.DeviceHandle, Sem = m_Semaphore]()
        {
            const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);

            if (Sem)
                VK_CALL(Device, vkDestroySemaphore(Device.DeviceVk, Sem, nullptr));
        });

        m_Semaphore = VK_NULL_HANDLE;
    }
}

C_STATUS CFenceVulkan::SignalFromDevice(CCommandQueue* CommandQueue, uint64 Value)
{
    CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
    const auto& Device = BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

    CCommandQueueVulkan* CommandQueueVk = BACKEND_DOWNCAST(CommandQueue, CCommandQueueVulkan);
    CASSERT(CommandQueueVk);

#if 0 // VkTimelineSemaphoreSubmitInfo is for older API prior to 1.3
    VkTimelineSemaphoreSubmitInfo TimeleineSemaphoreInfo{ .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
    TimeleineSemaphoreInfo.signalSemaphoreValueCount = 1;
    TimeleineSemaphoreInfo.pSignalSemaphoreValues = &Value;
#endif

    // Semaphore Signaling
    // The first synchronization scope includes every command submitted in the same batch
    // Semaphore signal operations that are defined by vkQueueSubmit or vkQueueSubmit2 
    // additionally include all commands that occur earlier in submission order.
    // Semaphore signal operations that are defined by vkQueueSubmit, vkQueueSubmit2 or vkQueueBindSparse 
    // additionally include in the first synchronization scope any
    // semaphore and fence signal operations that occur earlier in signal operation order.
    // The second synchronization scope includes only the semaphore signal operation.
    // The first access scope includes all memory access performed by the device.
    // The second access scope is empty.
    
    VkSemaphoreSubmitInfo SemaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
    SemaphoreInfo.semaphore = m_Semaphore;
    SemaphoreInfo.value = Value;
    SemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // #todo_vk optimize?

    VkSubmitInfo2 SubmitInfo{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
    SubmitInfo.signalSemaphoreInfoCount = 1;
    SubmitInfo.pSignalSemaphoreInfos = &SemaphoreInfo;
    
    VkResult Result = VK_CALL(Device, vkQueueSubmit2(CommandQueueVk->Get(), 1, &SubmitInfo, VK_NULL_HANDLE));
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_ERROR;
}

C_STATUS CFenceVulkan::SignalFromHost(uint64 Value)
{
    CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
    const auto& Device = BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

    VkSemaphoreSignalInfo Info{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO };
    Info.semaphore = m_Semaphore;
    Info.value = m_PendingValue;

    m_PendingValue = Value; // #todo_mt mutex here? or atomic CAS with return value something like OUTDATED
    VkResult Result = VK_CALL(Device, vkSignalSemaphore(Device.DeviceVk, &Info));
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CFenceVulkan::WaitFromHost(uint64 Value, uint64 Timeout)
{
    CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
    const auto& Device = BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

    VkSemaphoreWaitInfo Info{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
    Info.semaphoreCount = 1;
    Info.pSemaphores = &m_Semaphore;
    Info.pValues = &Value;

    VkResult Result = VK_CALL(Device, vkWaitSemaphores(Device.DeviceVk, &Info, Timeout));
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

uint64 CFenceVulkan::GetCompletedValue() const
{
    CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
    const auto& Device = BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);
    
    uint64 CompletedValue = 0;
    VkResult ResultVk = VK_CALL(Device, vkGetSemaphoreCounterValue(Device.DeviceVk, m_Semaphore, &CompletedValue));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, 0);

    return CompletedValue;
}

void CFenceVulkan::IncrementAndSignalFromHost()
{
    C_STATUS Result = SignalFromHost(++m_PendingValue);
    C_ASSERT_RETURN(C_SUCCEEDED(Result));
}

void CFenceVulkan::IncrementAndSignalFromDevice(CCommandQueue* CommandQueue)
{
    C_STATUS Result = SignalFromDevice(CommandQueue, ++m_PendingValue);
    C_ASSERT_RETURN(C_SUCCEEDED(Result));
}

} // namespace Cyclone::Render
