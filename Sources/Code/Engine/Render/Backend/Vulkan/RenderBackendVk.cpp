#include "RenderBackendVk.h"

#include "Common/CommonVulkan.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "CommandQueueVk.h"
#include "CommandBufferVk.h"

// #todo_vk
#include "DrawList.h"

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

    IApplication* App = m_Renderer->GetApp();
    m_WindowContexts.resize(App->GetWindowsCount());
    for (uint32 i = 0; i < App->GetWindowsCount(); ++i)
    {
        if (auto Window = App->GetWindow(i))
        {
            Result = m_WindowContexts[i].Init(this, Window);
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
        }
    }

    VkDescriptorPoolSize PSize{};
    PSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PSize.descriptorCount = 1000;

    VkDescriptorPoolCreateInfo DescPoolInfo{};
    DescPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    DescPoolInfo.maxSets = 1000;
    //DescPoolInfo.flags = VkDescriptorPoolCreateFlagBits:: ;
    DescPoolInfo.poolSizeCount = 1;
    DescPoolInfo.pPoolSizes = &PSize;
    VkResult ResultVk = vkCreateDescriptorPool(m_WindowContexts[0].GetLogicDevice(), &DescPoolInfo, nullptr, &m_DescriptorPool);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    DrawInit(this);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkan::Shutdown()
{
    // #todo_vk wait for GPU?

    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        auto& WindowContext = m_WindowContexts[i];
        WindowContext.Shutdown();
    }
    m_GlobalContext.Shutdown();

    return C_STATUS::C_STATUS_OK;
}
VkImageView RenderBackendVulkan::CreateImageView(DeviceHandle Device, VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectMask)
{
    VkImageViewCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    CreateInfo.image = image;
    CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    CreateInfo.format = format;
    CreateInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.subresourceRange.aspectMask = aspectMask;
    CreateInfo.subresourceRange.baseMipLevel = 0;
    CreateInfo.subresourceRange.levelCount = mipLevels;
    CreateInfo.subresourceRange.baseArrayLayer = 0;
    CreateInfo.subresourceRange.layerCount = 1;

    VkImageView ImageView;

    VkResult result = vkCreateImageView(GetGlobalContext().GetLogicalDevice(Device).LogicalDeviceHandle, &CreateInfo, nullptr, &ImageView);
    C_ASSERT_VK_SUCCEEDED(result);

    return ImageView;
}


C_STATUS RenderBackendVulkan::BeginRender()
{
    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        C_STATUS Result = m_WindowContexts[i].BeginRender();
        if (!C_SUCCEEDED(Result))
            return Result;
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkan::Render()
{
    Draw(this);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkan::EndRender()
{
    // #todo_vk temp
    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        CommandQueueVk* CommandQueue = m_WindowContexts[i].GetCommandQueue(CommandQueueType::Graphics);

        if (CommandBufferVk* CommandBuffer = CommandQueue->AllocateCommandBuffer())
        {
            CommandBuffer->Begin();
            CommandBuffer->End();

            auto CurrentFrame = m_WindowContexts[i].GetCurrentLocalFrame();
            CommandQueue->Submit(CommandBuffer, 1,
                m_WindowContexts[i].GetImageAvailableSemaphore(m_WindowContexts[i].GetCurrentLocalFrame()),
                m_WindowContexts[i].GetInflightFence(m_WindowContexts[i].GetCurrentLocalFrame()), true);
        }
    }
    // present
    for (uint32 i = 0; i < m_WindowContexts.size(); ++i)
    {
        C_STATUS Result = m_WindowContexts[i].Present(VK_NULL_HANDLE);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    m_CurrentFrame++;
    m_CurrentLocalFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return C_STATUS::C_STATUS_OK;
}

VkFormat RenderBackendVulkan::FindSupportedFormat(DeviceHandle Device, const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features)
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

VkFormat RenderBackendVulkan::FindDepthFormat(DeviceHandle Device)
{
    return FindSupportedFormat(Device, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool RenderBackendVulkan::hasStencilComponent(VkFormat Format)
{
    return Format == VK_FORMAT_D16_UNORM_S8_UINT
        || Format == VK_FORMAT_D24_UNORM_S8_UINT
        || Format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

} //namespace Cyclone::Render
