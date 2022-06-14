#include "FrameBufferVk.h"

#include "RenderBackendVulkan.h"
#include "RenderPassVk.h"

namespace Cyclone::Render
{

FrameBufferVk::~FrameBufferVk()
{
    DeInit();
    CASSERT(m_FrameBuffer == VK_NULL_HANDLE);
}

C_STATUS FrameBufferVk::Init(const FrameBufferVkInitInfo& InitInfo)
{
    CASSERT(m_FrameBuffer == VK_NULL_HANDLE);

    m_RenderPass = InitInfo.RenderPass;

    VkFramebufferCreateInfo FramebufferInfo{};
    FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferInfo.renderPass = m_RenderPass->Get();
    FramebufferInfo.attachmentCount = static_cast<uint32_t>(InitInfo.AttachmentsCount);
    FramebufferInfo.pAttachments = InitInfo.Attachments.data();
    FramebufferInfo.width = InitInfo.Width;
    FramebufferInfo.height = InitInfo.Height;
    FramebufferInfo.layers = InitInfo.Layers;

    VkDevice Device = InitInfo.Backend->GetGlobalContext().GetLogicalDevice(InitInfo.Device).LogicalDeviceHandle;
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
