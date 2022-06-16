#include "RenderPassVk.h"

#include "RenderBackendVulkan.h"
#include "CommandBufferVulkan.h"
#include "FrameBufferVk.h"

#include "Engine/Render/Common.h"
#include "Engine/Render/Types/Texture.h"

#define RENDER_PASS_MAX_ATTACHMENTS 10

namespace Cyclone::Render
{

C_STATUS RenderPassVk::Init(RenderBackendVulkan* Backend, CDeviceHandle DeviceHandle, const CRenderPass& RenderPass)
{
    CASSERT(m_RenderPass == VK_NULL_HANDLE);

    m_Backend = Backend;
    m_Device = DeviceHandle;

    const auto& RenderTargetSet = RenderPass.RenderTargetSet;
    bool HasDepthStencil = RenderTargetSet.DepthScentil.RenderTarget != nullptr;

    Array<VkAttachmentDescription, RENDER_PASS_MAX_ATTACHMENTS+1> Attachments{};

    Array<VkAttachmentReference, RENDER_PASS_MAX_ATTACHMENTS> ColorAttachmentsRefs{};
    VkAttachmentReference DepthStencilAttachmentRef{};

    Array<VkSubpassDescription, RENDER_PASS_MAX_ATTACHMENTS> Subpasses{};

    uint32_t ColorAttachmentRefCount = RenderTargetSet.RenderTargetsCount;
    uint32_t AttachmentCount = ColorAttachmentRefCount + (HasDepthStencil ? 1 : 0);
    uint32_t SubpassCount = 1;

    for (uint32 i = 0; i < RenderTargetSet.RenderTargetsCount; ++i)
    {
        auto RenderTargetBind = RenderTargetSet.RenderTargets[i];
        const auto& TextureDesc = RenderTargetBind.RenderTarget->Texture->GetDesc();

        VkAttachmentDescription& Attachment = Attachments[i];
        Attachment.format = ConvertFormatType(TextureDesc.Format);
        Attachment.samples = VK_SAMPLE_COUNT_1_BIT; // #todo_vk
        Attachment.loadOp = ConvertLoadOpType(RenderTargetBind.LoadOp);
        Attachment.storeOp = ConvertStoreOpType(RenderTargetBind.StoreOp);
        Attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        Attachment.initialLayout = ConvertLayoutType(RenderTargetBind.InitialLayout);
        Attachment.finalLayout = ConvertLayoutType(RenderTargetBind.FinalLayout);

        VkAttachmentReference& ColorAttachmentRef = ColorAttachmentsRefs[i];
        ColorAttachmentRef.attachment = i;
        ColorAttachmentRef.layout = ConvertLayoutType(RenderTargetBind.Layout);
    }

    if (HasDepthStencil && RenderTargetSet.DepthScentil.RenderTarget)
    {
        uint32 i = ColorAttachmentRefCount;
        auto RenderTargetBind = RenderTargetSet.DepthScentil;
        const auto& TextureDesc = RenderTargetBind.RenderTarget->Texture->GetDesc();

        VkAttachmentDescription& Attachment = Attachments[i];
        Attachment.format = ConvertFormatType(TextureDesc.Format);
        Attachment.samples = VK_SAMPLE_COUNT_1_BIT; // #todo_vk
        Attachment.loadOp = ConvertLoadOpType(RenderTargetBind.LoadOp);
        Attachment.storeOp = ConvertStoreOpType(RenderTargetBind.StoreOp);
        Attachment.stencilLoadOp = ConvertLoadOpType(RenderTargetBind.LoadOp);;
        Attachment.stencilStoreOp = ConvertStoreOpType(RenderTargetBind.StoreOp);
        Attachment.initialLayout = ConvertLayoutType(RenderTargetBind.InitialLayout);
        Attachment.finalLayout = ConvertLayoutType(RenderTargetBind.FinalLayout);

        VkAttachmentReference& AttachmentRef = DepthStencilAttachmentRef;
        AttachmentRef.attachment = i;
        AttachmentRef.layout = ConvertLayoutType(RenderTargetBind.Layout);
    }

    SubpassCount = 1;
    {
        VkSubpassDescription& Subpass = Subpasses[0];
        Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // #todo_vk
        Subpass.colorAttachmentCount = ColorAttachmentRefCount;
        Subpass.pColorAttachments = ColorAttachmentsRefs.data();
        Subpass.pDepthStencilAttachment = HasDepthStencil ? &DepthStencilAttachmentRef : nullptr;
    }

    // #todo_vk do external and internal dependency?
    uint32 SubpassDependencyCount = 2;
    Array<VkSubpassDependency, RENDER_PASS_MAX_ATTACHMENTS> SubpassDependencies{};
    {
        VkSubpassDependency& SubpassDependency = SubpassDependencies[0];
        SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        SubpassDependency.dstSubpass = 0;
        SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.srcAccessMask = 0;
        SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    {
        VkSubpassDependency& SubpassDependency = SubpassDependencies[1];
        SubpassDependency.srcSubpass = 0;
        SubpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.dstAccessMask = 0;
    }

    VkRenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = AttachmentCount;
    RenderPassInfo.pAttachments = Attachments.data();
    RenderPassInfo.subpassCount = SubpassCount;
    RenderPassInfo.pSubpasses = Subpasses.data();
    RenderPassInfo.dependencyCount = SubpassDependencyCount;
    RenderPassInfo.pDependencies = SubpassDependencies.data();

    VkDevice Device = Backend->GetGlobalContext().GetLogicalDevice(DeviceHandle).LogicalDeviceHandle;

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

void RenderPassVk::Begin(CommandBufferVulkan* CommandBuffer, FrameBufferVk* FrameBuffer, const CRenderPass& RenderPass)
{
    CASSERT(m_RenderPass != VK_NULL_HANDLE);
    m_FrameBuffer = FrameBuffer;

    VkRenderPassBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    BeginInfo.renderPass = m_RenderPass;
    BeginInfo.framebuffer = FrameBuffer->Get();

    BeginInfo.renderArea.offset = { (int32)RenderPass.ViewportExtent.X,  (int32)RenderPass.ViewportExtent.Y };
    BeginInfo.renderArea.extent = { (uint32)(RenderPass.ViewportExtent.Z - RenderPass.ViewportExtent.X), (uint32)(RenderPass.ViewportExtent.W - RenderPass.ViewportExtent.Y) };
    m_Viewport = BeginInfo.renderArea;
    m_ColorAttachmentCount = RenderPass.RenderTargetSet.RenderTargetsCount;

    uint32 ClearValueCount = RenderPass.RenderTargetSet.RenderTargetsCount;
    VkClearValue ClearValues[RENDER_PASS_MAX_ATTACHMENTS+1]{};
    for (uint32 i = 0; i < RenderPass.RenderTargetSet.RenderTargetsCount; ++i)
    {
        const auto& ClearValue = RenderPass.RenderTargetSet.RenderTargets[i].ClearValue;
        ClearValues[i].color = VkClearColorValue{
            ClearValue.Color.X, ClearValue.Color.Y, ClearValue.Color.Z, ClearValue.Color.W };
    }

    if (RenderPass.RenderTargetSet.DepthScentil.RenderTarget)
    {
        const auto& ClearValue = RenderPass.RenderTargetSet.DepthScentil.ClearValue;
        ClearValues[RenderPass.RenderTargetSet.RenderTargetsCount].depthStencil = VkClearDepthStencilValue{
            ClearValue.DepthStencil.X, static_cast<uint8>(ClearValue.DepthStencil.Y)};
        ClearValueCount++;
    }

    BeginInfo.clearValueCount = ClearValueCount;
    BeginInfo.pClearValues = ClearValues;

    // #todo_vk use new 1.3 feature vkCmdBeginRendering

    vkCmdBeginRenderPass(CommandBuffer->Get(), &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPassVk::End(CommandBufferVulkan* CommandBuffer)
{
    m_FrameBuffer = VK_NULL_HANDLE;

    CASSERT(m_RenderPass != VK_NULL_HANDLE);
    vkCmdEndRenderPass(CommandBuffer->Get());
}

RenderPassVk::~RenderPassVk()
{
    DeInit();
    CASSERT(m_RenderPass == VK_NULL_HANDLE);
}

} //namespace Cyclone::Render
