#include "RenderBackendVulkan.h"

#include "CommonVulkan.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "WindowContextVulkan.h"
#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"
#include "TextureVulkan.h"

namespace Cyclone::Render
{

CWindowContext* RenderBackendVulkan::CreateWindowContext(IWindow* Window)
{
    UniquePtr<WindowContextVulkan> Context = std::make_unique<WindowContextVulkan>();

    C_STATUS Result = Context->Init(m_Renderer, Window);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), nullptr);

    if (m_DescriptorPool == VK_NULL_HANDLE)
    {

        VkDescriptorPoolSize PSize{};
        PSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        PSize.descriptorCount = 1000;

        VkDescriptorPoolCreateInfo DescPoolInfo{};
        DescPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        DescPoolInfo.maxSets = 1000;
        //DescPoolInfo.flags = VkDescriptorPoolCreateFlagBits:: ;
        DescPoolInfo.poolSizeCount = 1;
        DescPoolInfo.pPoolSizes = &PSize;
        VkResult ResultVk = vkCreateDescriptorPool(GetGlobalContext().GetLogicalDevice({ 0,0 }).LogicalDeviceHandle, &DescPoolInfo, nullptr, &m_DescriptorPool);
        C_ASSERT_VK_SUCCEEDED(ResultVk);
    }

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


} //namespace Cyclone::Render
