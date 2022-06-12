#include "DrawList.h"

#include "RenderBackendVk.h"
#include "CommandQueueVk.h"
#include "CommandBufferVk.h"
#include "FrameBufferVk.h"
#include "RenderPassVk.h"

namespace Cyclone::Render
{

RenderPassVk GRenderPass[10];
FrameBufferVk GFrameBuffer[10][MAX_FRAMES_IN_FLIGHT + 1];

void DrawInit(RenderBackendVulkan* Backend)
{
    for (uint32 w = 0; w < Backend->GetWindowContextCount(); ++w)
    {
        auto& WindowContext = Backend->GetWindowContext(w);

        {
            RenderPassVkInitInfo RPInitInfo{};

            VkFormat ColorFormat = WindowContext.GetSwapchainImageFormat();

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
            RPInitInfo.Device = WindowContext.GetDevice();

            C_STATUS Result = GRenderPass[w].Init(RPInitInfo);
            CASSERT(C_SUCCEEDED(Result));
        }

        {
            FrameBufferVkInitInfo FBInitInfo{};
            FBInitInfo.Backend = Backend;
            FBInitInfo.RenderPass = &GRenderPass[w];
            FBInitInfo.Width = WindowContext.GetSwapchainExtent().width;
            FBInitInfo.Height = WindowContext.GetSwapchainExtent().height;
            FBInitInfo.Layers = 1;
            FBInitInfo.Device = WindowContext.GetDevice();

            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT + 1; ++i)
            {
                FBInitInfo.Attachments[0] = WindowContext.GetSwapchainImageView(i);// WindowContext.m_currentFrame];
                FBInitInfo.AttachmentsCount = 1;// 2;

                C_STATUS Result = GFrameBuffer[w][i].Init(FBInitInfo);
                CASSERT(C_SUCCEEDED(Result));
            }
        }
    }
}

void Draw(RenderBackendVulkan* Backend)
{
    for (uint32 i = 0; i < Backend->GetWindowContextCount(); ++i)
    {
        auto& WindowContext = Backend->GetWindowContext(i);
        // fill command buffers
        CommandQueueVk* CommandQueue = WindowContext.GetCommandQueue(CommandQueueType::Graphics);
        CASSERT(CommandQueue);

        CommandBufferVk* CommandBuffer = CommandQueue->AllocateCommandBuffer();
        CASSERT(CommandBuffer);

        CommandBuffer->Begin();

        {
            RenderPassVkBeginInfo RPBeginInfo{};
            RPBeginInfo.FrameBuffer = &GFrameBuffer[i][WindowContext.GetCurrentImageIndex()];
            RPBeginInfo.RenderArea.offset = { 0, 0 };
            RPBeginInfo.RenderArea.extent = WindowContext.GetSwapchainExtent();
            RPBeginInfo.ClearColors[0].color = { 0.f, 0.86f, 0.31f, 1.f };
            RPBeginInfo.ClearColors[1].depthStencil = { 1.0f, 0 };
            RPBeginInfo.ClearColorsCount = 1;// 2;

            GRenderPass[i].Begin(CommandBuffer, RPBeginInfo);

            // #todo_vk

            GRenderPass[i].End(CommandBuffer);
        }

        CommandBuffer->End();

        // submit
#if 0
        CommandQueue->Submit(CommandBuffer, 1,
            WindowContext.m_imageAvailabeSemaphores[Backend->GetWindowContext().m_currentFrame],
            Backend->GetWindowContext().m_inflightFences[WindowContext.m_currentFrame],
            true);
#else
        CommandQueue->Submit(CommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, true);
#endif
    }
}

} // namespace Cyclone::Render
