#include "FrameBufferVk.h"

#include "RenderBackendVk.h"
#include "RenderPassVk.h"

namespace Cyclone::Render
{

C_STATUS FrameBufferVk::Init(const FrameBufferVkInitInfo& InitInfo)
{
    m_renderPass = InitInfo.RenderPass;

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass->Get();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(InitInfo.AttachmentsCount);
    framebufferInfo.pAttachments = InitInfo.Attachments.data();
    framebufferInfo.width = InitInfo.Width;
    framebufferInfo.height = InitInfo.Height;
    framebufferInfo.layers = InitInfo.Layers;

    VkResult result = vkCreateFramebuffer(InitInfo.Backend->GetWindowContext().GetDevice(), &framebufferInfo, nullptr, &m_frameBuffer);
    C_ASSERT_VK_SUCCEEDED_RET(result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

} //namespace Cyclone::Render
