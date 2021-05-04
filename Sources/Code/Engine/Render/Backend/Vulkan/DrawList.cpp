#include "DrawList.h"

#include "RenderBackendVk.h"
#include "CommandQueueVk.h"
#include "CommandBufferVk.h"
#include "FrameBufferVk.h"
#include "RenderPassVk.h"

namespace Cyclone::Render
{

RenderPassVk g_renderPass;
FrameBufferVk g_frameBuffer[MAX_FRAMES_IN_FLIGHT + 1];
// DepthStencil!

void DrawInit(RenderBackendVulkan* Backend)
{
    {
        RenderPassVkInitInfo RPInitInfo{};

        VkFormat ColorFormat = Backend->GetWindowContext().m_swapchainImageFormat;

        RPInitInfo.ColorAttachmentCount = 1;
        VkAttachmentDescription& ColorAttachment = RPInitInfo.ColorAttachment[0];
        ColorAttachment.format = ColorFormat;
        ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        RPInitInfo.ColorAttachmentCount = 1;
        VkAttachmentReference& ColorAttachmentRef = RPInitInfo.ColorAttachmentRef[0];
        ColorAttachmentRef.attachment = 0;
        ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        RPInitInfo.SubpassCount = 1;
        VkSubpassDescription& Subpass = RPInitInfo.Subpass[0];
        Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        Subpass.colorAttachmentCount = 1;
        Subpass.pColorAttachments = &ColorAttachmentRef;

        RPInitInfo.SubpassDependencyCount = 1;
        VkSubpassDependency& SubpassDependency = RPInitInfo.SubpassDependency[0];
        SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        SubpassDependency.dstSubpass = 0;
        SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.srcAccessMask = 0;
        SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        RPInitInfo.Backend = Backend;

        C_STATUS Result = g_renderPass.Init(RPInitInfo);
        CASSERT(C_SUCCEEDED(Result));
    }

    {
        FrameBufferVkInitInfo FBInitInfo{};
        FBInitInfo.Backend = Backend;
        FBInitInfo.RenderPass = &g_renderPass;
        FBInitInfo.Width = Backend->GetWindowContext().m_swapchainExtent.width;
        FBInitInfo.Height = Backend->GetWindowContext().m_swapchainExtent.height;
        FBInitInfo.Layers = 1;

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT + 1; ++i)
        {
            FBInitInfo.Attachments[0] = Backend->GetWindowContext().m_swapchainImageViews[i];// Backend->GetWindowContext().m_currentFrame];
            FBInitInfo.AttachmentsCount = 1;// 2;

            C_STATUS Result = g_frameBuffer[i].Init(FBInitInfo);
            CASSERT(C_SUCCEEDED(Result));
        }
    }
}

void Draw(RenderBackendVulkan* Backend)
{
    //return;
    // fill command buffers
    CommandQueueVk* CommandQueue = Backend->GetWindowContext().GetCommandQueue(CommandQueueType::Graphics);
    CASSERT(CommandQueue);

    CommandBufferVk* CommandBuffer = CommandQueue->AllocateCommandBuffer();
    CASSERT(CommandBuffer);

    CommandBuffer->Begin();

    {
        RenderPassVkBeginInfo RPBeginInfo{};
        RPBeginInfo.FrameBuffer = &g_frameBuffer[Backend->GetWindowContext().m_currentImageIndex];
        RPBeginInfo.RenderArea.offset = { 0, 0 };
        RPBeginInfo.RenderArea.extent = Backend->GetWindowContext().m_swapchainExtent;
        RPBeginInfo.ClearColors[0].color = { 0.f, 0.86f, 0.31f, 1.f };
        RPBeginInfo.ClearColors[1].depthStencil = { 1.0f, 0 };
        RPBeginInfo.ClearColorsCount = 1;// 2;
        
        g_renderPass.Begin(CommandBuffer, RPBeginInfo);

        // #todo_vk

        g_renderPass.End(CommandBuffer);
    }

    CommandBuffer->End();

    // submit
#if 0
    CommandQueue->Submit(CommandBuffer, 1, 
        Backend->GetWindowContext().m_imageAvailabeSemaphores[Backend->GetWindowContext().m_currentFrame],
        Backend->GetWindowContext().m_inflightFences[Backend->GetWindowContext().m_currentFrame],
            true);
#else
    CommandQueue->Submit(CommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, true);
#endif
}

} // namespace Cyclone::Render
