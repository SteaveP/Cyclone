#include "RenderPassVk.h"

#include "RenderBackendVulkan.h"
#include "CommandBufferVulkan.h"
#include "FrameBufferVk.h"

#include "Engine/Render/Common.h"
#include "Engine/Render/Types/Texture.h"

namespace Cyclone::Render
{

C_STATUS RenderPassVk::Init(const RenderPassVkInitInfo& InitInfo)
{
    CASSERT(m_RenderPass == VK_NULL_HANDLE);

    m_Backend = InitInfo.Backend;
    m_SourceRenderPass = InitInfo.SourceRenderPass;
    m_Device = InitInfo.Device;

    VkDevice Device = InitInfo.Backend->GetGlobalContext().GetLogicalDevice(InitInfo.Device).LogicalDeviceHandle;
    
    VkRenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = static_cast<uint32_t>(InitInfo.ColorAttachmentCount);
    RenderPassInfo.pAttachments = InitInfo.ColorAttachment.data();
    RenderPassInfo.subpassCount = InitInfo.SubpassCount;
    RenderPassInfo.pSubpasses = InitInfo.Subpass.data();
    RenderPassInfo.dependencyCount = InitInfo.SubpassDependencyCount;
    RenderPassInfo.pDependencies = InitInfo.SubpassDependency.data();

    VkResult Result = vkCreateRenderPass(Device, &RenderPassInfo, nullptr, &m_RenderPass);
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void RenderPassVk::DeInit()
{
    if (m_RenderPass)
    {
        VkDevice Device = m_Backend->GetGlobalContext().GetLogicalDevice(m_Device).LogicalDeviceHandle;
        vkDestroyRenderPass(Device, m_RenderPass, nullptr);

        m_RenderPass = nullptr;
    }
}

void RenderPassVk::Begin(CommandBufferVulkan* CommandBuffer, const RenderPassVkBeginInfo& Info)
{
    CASSERT(m_RenderPass != VK_NULL_HANDLE);

    VkRenderPassBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    BeginInfo.renderPass = m_RenderPass;
    BeginInfo.framebuffer = Info.FrameBuffer->Get();
    BeginInfo.renderArea = Info.RenderArea;

    BeginInfo.clearValueCount = static_cast<uint32_t>(Info.ClearColors.size());
    BeginInfo.pClearValues = Info.ClearColors.data();

    vkCmdBeginRenderPass(CommandBuffer->Get(), &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPassVk::End(CommandBufferVulkan* CommandBuffer)
{
    CASSERT(m_RenderPass != VK_NULL_HANDLE);
    vkCmdEndRenderPass(CommandBuffer->Get());
}

RenderPassVk::~RenderPassVk()
{
    DeInit();
    CASSERT(m_RenderPass == VK_NULL_HANDLE);
}

void RenderPassVkInitInfo::CreateFromRenderPass(RenderPassVkInitInfo& Desc, RenderBackendVulkan* Backend, DeviceHandle Device, const CRenderPass& RenderPass)
{
    const auto& RenderTargetSet = RenderPass.RenderTargetSet;

    bool HasDepthStencil = RenderTargetSet.DepthScentil.RenderTarget != nullptr;
    Desc.ColorAttachmentCount = RenderTargetSet.RenderTargetsCount + (HasDepthStencil ? 1 : 0);
    Desc.ColorAttachmentRefCount = Desc.ColorAttachmentCount;

    for (uint32 i = 0; i < RenderTargetSet.RenderTargetsCount; ++i)
    {
        auto RenderTargetBind = RenderTargetSet.RenderTargets[i];
        const auto& TextureDesc = RenderTargetBind.RenderTarget->Texture->GetDesc();
        VkAttachmentDescription& ColorAttachment = Desc.ColorAttachment[i];
        ColorAttachment.format = ConvertFormatType(TextureDesc.Format);
        ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // #todo_vk
        ColorAttachment.loadOp = RenderTargetBind.ClearValue.has_value() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference& ColorAttachmentRef = Desc.ColorAttachmentRef[i];
        ColorAttachmentRef.attachment = i;
        ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    // Depth Attachments #todo_vk refactor
    if (RenderTargetSet.DepthScentil.RenderTarget)
    {
        uint32 i = RenderTargetSet.RenderTargetsCount;
        auto RenderTargetBind = RenderTargetSet.DepthScentil;
        const auto& TextureDesc = RenderTargetBind.RenderTarget->Texture->GetDesc();
        VkAttachmentDescription& ColorAttachment = Desc.ColorAttachment[i];
        ColorAttachment.format = ConvertFormatType(TextureDesc.Format);
        ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // #todo_vk
        ColorAttachment.loadOp = RenderTargetBind.ClearValue.has_value() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference& ColorAttachmentRef = Desc.ColorAttachmentRef[i];
        ColorAttachmentRef.attachment = i;
        ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    Desc.SubpassCount = 1;
    VkSubpassDescription& Subpass = Desc.Subpass[0];
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpass.colorAttachmentCount = Desc.ColorAttachmentRefCount;
    Subpass.pColorAttachments = Desc.ColorAttachmentRef.data();

    Desc.SubpassDependencyCount = 1;
    VkSubpassDependency& SubpassDependency = Desc.SubpassDependency[0];
    SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependency.dstSubpass = 0;
    SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.srcAccessMask = 0;
    SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    Desc.Backend = Backend;
    Desc.Device = Device;
    Desc.SourceRenderPass = &RenderPass;
}

} //namespace Cyclone::Render
