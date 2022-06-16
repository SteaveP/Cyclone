#include "RenderBackendVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "CommonVulkan.h"
#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"

#include "Internal/ResourceManagerVk.h"

namespace Cyclone::Render
{

C_STATUS RenderBackendVulkan::Init(IRenderer* Renderer)
{
    m_Renderer = Renderer;

    C_STATUS Result = C_STATUS::C_STATUS_OK;
    {
        InstanceCreationDesc Desc{};

        Result = m_GlobalContext.Init(this, Desc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkan::Shutdown()
{
    WaitGPU();

    m_GlobalContext.Shutdown();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkan::BeginRender()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkan::EndRender()
{
    return C_STATUS::C_STATUS_OK;
}

VkFormat RenderBackendVulkan::FindSupportedFormat(CDeviceHandle Device, const Vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features)
{
    for (VkFormat Format : Candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_GlobalContext.GetPhysicalDevice(Device).PhysicalDeviceHandle, Format, &props);

        if (Tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & Features) == Features)
            return Format;
        else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & Features) == Features)
            return Format;
    }

    CASSERT(false); // unsupported
    return VK_FORMAT_UNDEFINED;
}

VkFormat RenderBackendVulkan::FindDepthFormat(CDeviceHandle Device)
{
    return FindSupportedFormat(Device, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool RenderBackendVulkan::HasStencilComponent(VkFormat Format)
{
    return Format == VK_FORMAT_D16_UNORM_S8_UINT
        || Format == VK_FORMAT_D24_UNORM_S8_UINT
        || Format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

void RenderBackendVulkan::WaitGPU()
{
    // #todo_vk wait all devices
    VkResult Result = vkDeviceWaitIdle(m_GlobalContext.GetLogicalDevice({ 0,0 }).LogicalDeviceHandle);
    C_ASSERT_VK_SUCCEEDED(Result);
}

CResourceManagerVk* RenderBackendVulkan::GetResourceManager(CDeviceHandle Device)
{
    auto& LogicDevice = GetGlobalContext().GetLogicalDevice(Device);
    return LogicDevice.ResourceManager.get();
}

} //namespace Cyclone::Render
