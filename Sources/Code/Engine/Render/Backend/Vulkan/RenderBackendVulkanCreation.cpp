#include "RenderBackendVulkan.h"

#include "CommonVulkan.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "WindowContextVulkan.h"
#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"
#include "TextureVulkan.h"
#include "BufferVulkan.h"
#include "ShaderVulkan.h"

namespace Cyclone::Render
{

CWindowContext* RenderBackendVulkan::CreateWindowContext(IWindow* Window)
{
    UniquePtr<WindowContextVulkan> Context = MakeUnique<WindowContextVulkan>();

    C_STATUS Result = Context->Init(m_Renderer, Window);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), nullptr);

    return Context.release();
}

CCommandQueue* RenderBackendVulkan::CreateCommandQueue()
{
    return new CommandQueueVulkan();
}

CCommandBuffer* RenderBackendVulkan::CreateCommandBuffer()
{
    return new CommandBufferVulkan();
}

CTexture* RenderBackendVulkan::CreateTexture()
{
    return new CTextureVulkan();
}

CShader* RenderBackendVulkan::CreateShader()
{
    return new CShaderVulkan();
}

CPipeline* RenderBackendVulkan::CreatePipeline()
{
    return nullptr;
}

RawPtr RenderBackendVulkan::CreateTextureView(CDeviceHandle Device, CTexture* Texture, uint32 StartMip, uint32 MipLevels, EFormatType Format, EImageAspectType AspectMask)
{
    auto TextureVk = BACKEND_DOWNCAST(Texture, CTextureVulkan);
    return CreateImageView(Device, TextureVk->GetImage(), 1, ConvertFormatType(TextureVk->GetDesc().Format), ConvertImageAspectType(AspectMask));
}

CBuffer* RenderBackendVulkan::CreateBuffer()
{
    return new CBufferVulkan();
}

RawPtr RenderBackendVulkan::CreateBufferView(CDeviceHandle Device, CBuffer* Buffer, uint64 Offset, uint64 Size, EFormatType Format)
{
    auto BufferVk = BACKEND_DOWNCAST(Buffer, CBufferVulkan);
    return CreateBufferView(Device, BufferVk->GetBuffer(), Offset, Size, ConvertFormatType(BufferVk->GetDesc().Format));
}

VkImageView RenderBackendVulkan::CreateImageView(CDeviceHandle Device, VkImage Image, uint32 MipLevels, VkFormat Format, VkImageAspectFlags AspectMask)
{
    // #todo_vk refactor
    VkImageViewCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    CreateInfo.image = Image;
    CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    CreateInfo.format = Format;
    CreateInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
    CreateInfo.subresourceRange.aspectMask = AspectMask;
    CreateInfo.subresourceRange.baseMipLevel = 0;
    CreateInfo.subresourceRange.levelCount = MipLevels;
    CreateInfo.subresourceRange.baseArrayLayer = 0;
    CreateInfo.subresourceRange.layerCount = 1;

    VkImageView ImageView;

    VkResult result = vkCreateImageView(GetGlobalContext().GetLogicalDevice(Device).LogicalDeviceHandle, &CreateInfo, nullptr, &ImageView);
    C_ASSERT_VK_SUCCEEDED(result);

    return ImageView;
}

VkBufferView RenderBackendVulkan::CreateBufferView(CDeviceHandle Device, VkBuffer Buffer, uint64 Offset, uint64 Size, VkFormat Format)
{
    VkBufferViewCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    CreateInfo.flags;
    CreateInfo.format;
    CreateInfo.offset = Offset;
    CreateInfo.range = Size;

    CreateInfo.buffer = Buffer;
    CreateInfo.format = Format;

    VkBufferView BufferView;

    VkResult Result = vkCreateBufferView(GetGlobalContext().GetLogicalDevice(Device).LogicalDeviceHandle, &CreateInfo, nullptr, &BufferView);
    C_ASSERT_VK_SUCCEEDED(Result);

    return BufferView;
}

void RenderBackendVulkan::DestroyTextureView(CDeviceHandle Device, CTexture* Texture, RawPtr TextureView)
{
    auto TextureVk = BACKEND_DOWNCAST(Texture, CTextureVulkan);
    vkDestroyImageView(GetGlobalContext().GetLogicalDevice(Device).LogicalDeviceHandle, reinterpret_cast<VkImageView>(TextureView), nullptr);
}

void RenderBackendVulkan::DestroyBufferView(CDeviceHandle Device, CBuffer* Buffer, RawPtr BufferView)
{
    auto BufferVk = BACKEND_DOWNCAST(Buffer, CBufferVulkan);
    vkDestroyBufferView(GetGlobalContext().GetLogicalDevice(Device).LogicalDeviceHandle, reinterpret_cast<VkBufferView>(BufferView), nullptr);
}


} //namespace Cyclone::Render
