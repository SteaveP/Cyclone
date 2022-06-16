#include "FrameBufferVk.h"

#include "RenderBackendVulkan.h"
#include "RenderPassVk.h"

#include "Engine/Render/Types/Texture.h"

namespace Cyclone::Render
{

FrameBufferVk::~FrameBufferVk()
{
    DeInit();
    CASSERT(m_FrameBuffer == VK_NULL_HANDLE);
}

C_STATUS FrameBufferVk::Init(CDeviceHandle DeviceHandle, RenderBackendVulkan* BackendVk, RenderPassVk* RenderPassVkPtr, const CRenderPass& RenderPass)
{
    CASSERT(m_FrameBuffer == VK_NULL_HANDLE);
    m_RenderPass = RenderPassVkPtr;

    CTexture* TextureRef = nullptr;
    if (RenderPass.RenderTargetSet.RenderTargetsCount > 0)
        TextureRef = RenderPass.RenderTargetSet.RenderTargets[0].RenderTarget->Texture.get();
    if (TextureRef == nullptr && RenderPass.RenderTargetSet.DepthScentil.RenderTarget)
        TextureRef = RenderPass.RenderTargetSet.DepthScentil.RenderTarget->Texture.get();
    CASSERT(TextureRef);

    VkFramebufferCreateInfo FramebufferInfo{};
    FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferInfo.renderPass = RenderPassVkPtr->Get();

    VkImageView Attachments[10]{};
    uint32 AttachmentCount = RenderPass.RenderTargetSet.RenderTargetsCount 
        + (RenderPass.RenderTargetSet.DepthScentil.RenderTarget ? 1 : 0);

    for (uint32 i = 0; i < RenderPass.RenderTargetSet.RenderTargetsCount; ++i)
    {
        Attachments[i] = (VkImageView)RenderPass.RenderTargetSet.RenderTargets[i].RenderTarget->RenderTargetView->PlatformDataPtr;
    }
    if (RenderPass.RenderTargetSet.DepthScentil.RenderTarget)
    {
        Attachments[RenderPass.RenderTargetSet.RenderTargetsCount] = (VkImageView)RenderPass.RenderTargetSet.DepthScentil.RenderTarget->RenderTargetView->PlatformDataPtr;
    }

    FramebufferInfo.attachmentCount = AttachmentCount;
    FramebufferInfo.pAttachments = Attachments;

    FramebufferInfo.width = static_cast<uint32>(TextureRef->GetDesc().Width);
    FramebufferInfo.height = TextureRef->GetDesc().Height;
    FramebufferInfo.layers = 1;

    m_RenderPass = RenderPassVkPtr;

    VkDevice Device = BackendVk->GetGlobalContext().GetLogicalDevice(DeviceHandle).LogicalDeviceHandle;
    VkResult Result = vkCreateFramebuffer(Device, &FramebufferInfo, nullptr, &m_FrameBuffer);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void FrameBufferVk::DeInit()
{
    if (m_FrameBuffer)
    {
        VkDevice Device = m_RenderPass->GetBackend()->GetGlobalContext().GetLogicalDevice(m_RenderPass->GetDevice()).LogicalDeviceHandle;
        vkDestroyFramebuffer(Device, m_FrameBuffer, nullptr);

        m_RenderPass = nullptr;
        m_FrameBuffer = VK_NULL_HANDLE;
    }
}

} //namespace Cyclone::Render
