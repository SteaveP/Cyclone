#include "RenderBackendVulkan.h"

#include "Engine/Core/Math.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/Utils/Delegate.h"
#include "Engine/Utils/Config.h"

#include "CommonVulkan.h"
#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"
#include "WindowContextVulkan.h"

#include "ResourceManagerVulkan.h"

namespace Cyclone::Render
{

CRenderBackendVulkan::CRenderBackendVulkan() = default;
CRenderBackendVulkan::~CRenderBackendVulkan()
{
    ShutdownImpl();
}

C_STATUS CRenderBackendVulkan::Init(IRenderer* Renderer)
{
    m_Renderer = Renderer;

    C_STATUS Result = C_STATUS::C_STATUS_OK;
    {
        CInstanceCreationDesc Desc{};
        Desc.EnabledLayers = GET_CONFIG_VULKAN().value("InstanceLayers", Desc.EnabledLayers);
        Desc.EnabledExtensions = GET_CONFIG_VULKAN().value("InstanceExtensions", Desc.EnabledExtensions);

        Result = m_DeviceManager.Init(this, Desc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CRenderBackendVulkan::Shutdown()
{
    ShutdownImpl();
    return C_STATUS::C_STATUS_OK;
}

void CRenderBackendVulkan::ShutdownImpl() noexcept
{
    WaitGPU();

    m_WindowContextPool = {};
    m_DeviceManager.Shutdown();
}

C_STATUS CRenderBackendVulkan::BeginRender()
{
    for (uint32 i = 0; i < m_DeviceManager.GetDeviceCount(); ++i)
    {
        auto& Device = m_DeviceManager.GetDevice(CDeviceHandle(i, 0));
        if (Device.DisposalManager)
            Device.DisposalManager->Tick();
    }
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CRenderBackendVulkan::EndRender()
{
    for (uint32 i = 0; i < m_DeviceManager.GetDeviceCount(); ++i)
    {
        auto& Device = m_DeviceManager.GetDevice(CDeviceHandle(i, 0));
        if (Device.DisposalManager)
            Device.DisposalManager->Tick();
    }
    return C_STATUS::C_STATUS_OK;
}

VkFormat CRenderBackendVulkan::FindSupportedFormat(CDeviceHandle Device, const Vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features)
{
    for (VkFormat Format : Candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_DeviceManager.GetPhysDevice(Device).DeviceVk, Format, &props);

        if (Tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & Features) == Features)
            return Format;
        else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & Features) == Features)
            return Format;
    }

    CASSERT(false); // unsupported
    return VK_FORMAT_UNDEFINED;
}

VkFormat CRenderBackendVulkan::FindDepthFormat(CDeviceHandle Device)
{
    return FindSupportedFormat(Device, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool CRenderBackendVulkan::HasStencilComponent(VkFormat Format)
{
    return Format == VK_FORMAT_D16_UNORM_S8_UINT
        || Format == VK_FORMAT_D24_UNORM_S8_UINT
        || Format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

void CRenderBackendVulkan::WaitGPU()
{
    for(uint32 i = 0; i < (uint32)m_DeviceManager.GetDeviceCount(); ++i)
    {
        auto& Device = m_DeviceManager.GetDevice( CDeviceHandle{ i, 0 });
        VkResult Result = VK_CALL(Device, vkDeviceWaitIdle(Device.DeviceVk));
        C_ASSERT_VK_SUCCEEDED(Result);
    }
}

CResourceManagerVulkan* CRenderBackendVulkan::GetResourceManagerVk(CDeviceHandle DeviceHandle)
{
    auto& Device = GetDeviceManager().GetDevice(DeviceHandle);
    return Device.ResourceManager.get();
}

CDisposalManagerVulkan* CRenderBackendVulkan::GetDisposalManagerVk(CDeviceHandle DeviceHandle)
{
    auto& Device = GetDeviceManager().GetDevice(DeviceHandle);
    return Device.DisposalManager.get();
}

void CRenderBackendVulkan::ProfileGPUEventBegin(class CCommandBuffer* CommandBuffer, const char* Name, const Vec4& Color)
{
#if ENABLE_DEBUG_RENDER_BACKEND
    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);

    const auto& Device = CommandBufferVk->GetBackendVk()->GetDeviceManager().GetDevice(CommandBufferVk->GetDeviceHandle());

    if (Device.DeviceTableExt.vkQueueBeginDebugUtilsLabelEXT)
    {
        VkDebugUtilsLabelEXT Label{ .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = Name,
            .color = {Color.X, Color.Y, Color.Z, Color.W}
        };

        Device.DeviceTableExt.vkCmdBeginDebugUtilsLabelEXT(CommandBufferVk->Get(), &Label);
    }
#endif
}

void CRenderBackendVulkan::ProfileGPUEventBegin(CCommandQueue* CommandQueue, const char* Name, const Vec4& Color)
{
#if ENABLE_DEBUG_RENDER_BACKEND
    CCommandQueueVulkan* CommandQueueVk = BACKEND_DOWNCAST(CommandQueue, CCommandQueueVulkan);

    const auto& LogicDevice = CommandQueueVk->
        GetBackendVk()->GetDeviceManager().GetDevice(CommandQueueVk->GetDeviceHandle());

    if (LogicDevice.DeviceTableExt.vkQueueBeginDebugUtilsLabelEXT)
    {
        VkDebugUtilsLabelEXT Label{ .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = Name,
            .color = {Color.X, Color.Y, Color.Z, Color.W}
        };

        LogicDevice.DeviceTableExt.vkQueueBeginDebugUtilsLabelEXT(CommandQueueVk->Get(), &Label);
    }
#endif
}

void CRenderBackendVulkan::ProfileGPUEventEnd(class CCommandBuffer* CommandBuffer)
{
#if ENABLE_DEBUG_RENDER_BACKEND
    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);

    const auto& Device = GetDeviceManager().GetDevice(CommandBuffer->GetDeviceHandle());

    CASSERT(Device.DeviceTableExt.vkCmdEndDebugUtilsLabelEXT);
    if (Device.DeviceTableExt.vkCmdEndDebugUtilsLabelEXT)
    {
        Device.DeviceTableExt.vkCmdEndDebugUtilsLabelEXT(CommandBufferVk->Get());
    }
#endif
}

void CRenderBackendVulkan::ProfileGPUEventEnd(CCommandQueue* CommandQueue)
{
#if ENABLE_DEBUG_RENDER_BACKEND
    CCommandQueueVulkan* CommandQueueVk = BACKEND_DOWNCAST(CommandQueue, CCommandQueueVulkan);

    const auto& Device = GetDeviceManager().GetDevice(CommandQueue->GetDeviceHandle());

    CASSERT(Device.DeviceTableExt.vkQueueEndDebugUtilsLabelEXT);
    if (Device.DeviceTableExt.vkQueueEndDebugUtilsLabelEXT)
    {
        Device.DeviceTableExt.vkQueueEndDebugUtilsLabelEXT(CommandQueueVk->Get());
    }
#endif
}

CBindlessDescriptorManagerVulkan* CRenderBackendVulkan::GetBindlessManagerVk(CDeviceHandle DeviceHandle)
{
    return GetDeviceManager().GetDevice(DeviceHandle).BindlessManager.get();
}

CUploadQueue* CRenderBackendVulkan::GetUploadQueue(CDeviceHandle DeviceHandle)
{
    return GetDeviceManager().GetDevice(DeviceHandle).UploadQueue.get();
}

CHandle<CWindowContext> CRenderBackendVulkan::CreateWindowContext()
{
    CHandle<CWindowContext> Handle = m_WindowContextPool.Create();
    return Handle;
}

void CRenderBackendVulkan::DestroyWindowContext(CHandle<CWindowContext> Handle)
{
    m_WindowContextPool.Destroy(MoveTemp(Handle));
}

CWindowContext* CRenderBackendVulkan::GetWindowContext(CHandle<CWindowContext> Handle)
{
    return m_WindowContextPool.Get(MoveTemp(Handle));
}

#if ENABLE_DEBUG_RENDER_BACKEND
bool SetDebugNameVk(std::string_view Name, VkObjectType Type, uint64 Handle, const CDevice& Device)
{
    if (Device.DeviceTableExt.vkSetDebugUtilsObjectNameEXT)
    {
        VkDebugUtilsObjectNameInfoEXT DebugInfo{};
        DebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        DebugInfo.objectType = Type;
        DebugInfo.objectHandle = Handle;
        DebugInfo.pObjectName = Name.data();
        VkResult Result = Device.DeviceTableExt.vkSetDebugUtilsObjectNameEXT(Device.DeviceVk, &DebugInfo);
        C_ASSERT_VK_SUCCEEDED_RET(Result, false);
    }

    return true;
}
#endif

} //namespace Cyclone::Render
