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
    m_renderer = Renderer;

    C_STATUS result = m_globalContext.Init(this);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    result = m_windowContext.Init(this, m_renderer->GetApp()->GetWindow());
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    VkDescriptorPoolSize PSize{};
    PSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PSize.descriptorCount = 1000;

    VkDescriptorPoolCreateInfo DescPoolInfo{};
    DescPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    DescPoolInfo.maxSets = 1000;
    //DescPoolInfo.flags = VkDescriptorPoolCreateFlagBits:: ;
    DescPoolInfo.poolSizeCount = 1;
    DescPoolInfo.pPoolSizes = &PSize;
    VkResult ResultVk = vkCreateDescriptorPool(m_windowContext.GetDevice(), &DescPoolInfo, nullptr, &m_descriptorPool);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    DrawInit(this);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkan::Shutdown()
{
    // #todo_vk wait?

    m_windowContext.Shutdown();
    m_globalContext.Shutdown();

    return C_STATUS::C_STATUS_OK;
}
VkImageView RenderBackendVulkan::CreateImageView(VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectMask)
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectMask;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;

    VkResult result = vkCreateImageView(m_windowContext.GetDevice(), &createInfo, nullptr, &imageView);
    C_ASSERT_VK_SUCCEEDED(result);

    return imageView;
}


C_STATUS RenderBackendVulkan::BeginRender()
{
    return m_windowContext.BeginRender();
}

C_STATUS RenderBackendVulkan::Render()
{
    Draw(this);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkan::EndRender()
{
    // present
    C_STATUS Result = m_windowContext.Present(VK_NULL_HANDLE);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_currentFrame++;
    m_currentLocalFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return C_STATUS::C_STATUS_OK;
}

VkFormat RenderBackendVulkan::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_windowContext.GetPhysDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }

    CASSERT(false); // unsupported
    return VK_FORMAT_UNDEFINED;
}

VkFormat RenderBackendVulkan::FindDepthFormat()
{
    return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool RenderBackendVulkan::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM_S8_UINT
        || format == VK_FORMAT_D24_UNORM_S8_UINT
        || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

} //namespace Cyclone::Render
