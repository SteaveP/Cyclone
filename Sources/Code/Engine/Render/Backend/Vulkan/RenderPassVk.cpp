#include "RenderPassVk.h"

#include "RenderBackendVk.h"
#include "CommandBufferVk.h"
#include "FrameBufferVk.h"

namespace Cyclone::Render
{

C_STATUS RenderPassVk::Init(const RenderPassVkInitInfo& InitInfo)
{
    m_Backend = InitInfo.Backend;
    VkDevice Device = m_Backend->GetGlobalContext().GetLogicalDevice(InitInfo.Device).LogicalDeviceHandle;
    
    VkRenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = static_cast<uint32_t>(InitInfo.ColorAttachmentCount);
    RenderPassInfo.pAttachments = InitInfo.ColorAttachment.data();
    RenderPassInfo.subpassCount = InitInfo.SubpassCount;
    RenderPassInfo.pSubpasses = InitInfo.Subpass.data();
    RenderPassInfo.dependencyCount = InitInfo.SubpassDependencyCount;
    RenderPassInfo.pDependencies = InitInfo.SubpassDependency.data();

    VkResult Result = vkCreateRenderPass(Device, &RenderPassInfo, nullptr, &m_renderPass);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void RenderPassVk::Begin(CommandBufferVk* CommandBuffer, const RenderPassVkBeginInfo& Info)
{
    CASSERT(m_renderPass != VK_NULL_HANDLE);

    VkRenderPassBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    BeginInfo.renderPass = m_renderPass;
    BeginInfo.framebuffer = Info.FrameBuffer->Get();
    BeginInfo.renderArea = Info.RenderArea;

    BeginInfo.clearValueCount = static_cast<uint32_t>(Info.ClearColors.size());
    BeginInfo.pClearValues = Info.ClearColors.data();

    vkCmdBeginRenderPass(CommandBuffer->Get(), &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPassVk::End(CommandBufferVk* CommandBuffer)
{
    CASSERT(m_renderPass != VK_NULL_HANDLE);
    vkCmdEndRenderPass(CommandBuffer->Get());
}

} //namespace Cyclone::Render
