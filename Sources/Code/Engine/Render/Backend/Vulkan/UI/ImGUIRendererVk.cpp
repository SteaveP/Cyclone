#include "ImGUIRendererVk.h"

#include "../Common/CommonVulkan.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"

#include "Engine/UI/ImGui/CommonImGui.h"
#include <backends/imgui_impl_vulkan.h>
#include "../RenderBackendVk.h"
#include "../CommandQueueVk.h"
#include "../CommandBufferVk.h"
#include "../RenderPassVk.h"
#include "../FrameBufferVk.h"

//#define DISABLE_IMGUI 1

namespace Cyclone::Render
{

    class ImGUIRendererVk::Pimpl
    {
    public:
        RenderBackendVulkan* Backend;
        RenderPassVk RenderPassUI;
        std::vector<FrameBufferVk> FrameBufferUI;
    };

    ImGUIRendererVk::ImGUIRendererVk() = default;
    ImGUIRendererVk::~ImGUIRendererVk()
    {
        delete m_pimpl;
    };

    Cyclone::C_STATUS ImGUIRendererVk::OnInit(void* Instance, IUIModule* UIModule, Render::IRendererBackend* IBackend, IWindow* window)
    {
        delete m_pimpl;
        m_pimpl = new Pimpl();

        m_module = UIModule;

        m_pimpl->Backend = dynamic_cast<RenderBackendVulkan*>(IBackend);
        C_ASSERT_RETURN_VAL(m_pimpl->Backend, C_STATUS::C_STATUS_INVALID_ARG);

        RenderBackendVulkan* Backend = m_pimpl->Backend;
        WindowContextVk& WindowContext = Backend->GetWindowContext();
        {
            RenderPassVkInitInfo RPInitInfo{};

            VkFormat ColorFormat = WindowContext.m_swapchainImageFormat;

            RPInitInfo.ColorAttachmentCount = 1;
            VkAttachmentDescription& ColorAttachment = RPInitInfo.ColorAttachment[0];
            ColorAttachment.format = ColorFormat;
            ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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

            C_STATUS Result = m_pimpl->RenderPassUI.Init(RPInitInfo);
            CASSERT(C_SUCCEEDED(Result));
        }

        {
            FrameBufferVkInitInfo FBInitInfo{};
            FBInitInfo.Backend = Backend;
            FBInitInfo.RenderPass = &m_pimpl->RenderPassUI;
            FBInitInfo.Width = WindowContext.m_swapchainExtent.width;
            FBInitInfo.Height = WindowContext.m_swapchainExtent.height;
            FBInitInfo.Layers = 1;

            m_pimpl->FrameBufferUI.resize(WindowContext.m_swapchainImageViews.size());
            for (uint32_t i = 0; i < m_pimpl->FrameBufferUI.size(); ++i)
            {
                FBInitInfo.Attachments[0] = WindowContext.m_swapchainImageViews[i];
                FBInitInfo.AttachmentsCount = 1;

                C_STATUS Result = m_pimpl->FrameBufferUI[i].Init(FBInitInfo);
                CASSERT(C_SUCCEEDED(Result));
            }
        }

#ifndef DISABLE_IMGUI
        ImGuiContext* Context = reinterpret_cast<ImGuiContext*>(Instance);
        ImGui::SetCurrentContext(Context);

        ImGui_ImplVulkan_InitInfo InitInfo{};
        InitInfo.Instance = Backend->GetGlobalContext().GetInstance();
        InitInfo.PhysicalDevice = WindowContext.GetPhysDevice();
        InitInfo.Device = WindowContext.GetDevice();
        InitInfo.Queue = WindowContext.GetCommandQueue(CommandQueueType::Graphics)->Get();
        InitInfo.QueueFamily = WindowContext.GetCommandQueue(CommandQueueType::Graphics)->GetQueueFamilyIndex();
        InitInfo.PipelineCache = VK_NULL_HANDLE;
        InitInfo.DescriptorPool = Backend->GetDescriptorPool();
        InitInfo.Subpass = 0;
        InitInfo.MinImageCount = WindowContext.m_minSwapchainImageCount;
        InitInfo.ImageCount = (uint32_t)WindowContext.m_swapchainImages.size();
        InitInfo.MSAASamples = WindowContext.m_currentMsaaSamples;

        bool Result = ImGui_ImplVulkan_Init(&InitInfo, m_pimpl->RenderPassUI.Get());
        C_ASSERT_RETURN_VAL(Result, C_STATUS::C_STATUS_ERROR);

        {
            CommandQueueVk* CommandQueue = WindowContext.GetCommandQueue(CommandQueueType::Graphics);
            CASSERT(CommandQueue);

            CommandBufferVk* CommandBuffer = CommandQueue->AllocateCommandBuffer();
            CASSERT(CommandBuffer);

            CommandBuffer->Begin();

            ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer->Get());

            CommandBuffer->End();

            C_STATUS Res = CommandQueue->Submit(CommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, true);
            C_ASSERT_RETURN_VAL(C_SUCCEEDED(Res), Res);

            // wait for the completion
            VkResult ResultVk = vkQueueWaitIdle(CommandQueue->Get());
            C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);
        }
#endif

        return C_STATUS::C_STATUS_OK;
    }

    C_STATUS ImGUIRendererVk::OnFrame(void* Instance)
    {
#ifndef DISABLE_IMGUI
        ImGui_ImplVulkan_NewFrame();
#endif

        return C_STATUS::C_STATUS_OK;
    }

    C_STATUS ImGUIRendererVk::OnRender(void* Instance)
    {
#ifndef DISABLE_IMGUI
        //PROFILE_GPU_SCOPED_EVENT(commandList->GetCommandList(), "ImGUI Render");

        RenderBackendVulkan* Backend = m_pimpl->Backend;
        WindowContextVk& WindowContext = Backend->GetWindowContext();

        CommandQueueVk* CommandQueue = WindowContext.GetCommandQueue(CommandQueueType::Graphics);
        CASSERT(CommandQueue);

        CommandBufferVk* CommandBuffer = CommandQueue->AllocateCommandBuffer();
        CASSERT(CommandBuffer);

        CommandBuffer->Begin();

        {
            RenderPassVkBeginInfo RPBeginInfo{};
            RPBeginInfo.FrameBuffer = &m_pimpl->FrameBufferUI[WindowContext.m_currentImageIndex];
            RPBeginInfo.RenderArea.offset = { 0, 0 };
            RPBeginInfo.RenderArea.extent = WindowContext.m_swapchainExtent;

            m_pimpl->RenderPassUI.Begin(CommandBuffer, RPBeginInfo);
        }

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer->Get());

        m_pimpl->RenderPassUI.End(CommandBuffer);

        CommandBuffer->End();

        // submit
#if 1
        CommandQueue->Submit(CommandBuffer, 1,
            WindowContext.m_imageAvailabeSemaphores[WindowContext.m_currentFrame],
            WindowContext.m_inflightFences[WindowContext.m_currentFrame], true);
#else
        CommandQueue->Submit(CommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, true);
#endif
#endif

        return C_STATUS::C_STATUS_OK;
    }

    C_STATUS ImGUIRendererVk::OnShutdown(void* Instance, IWindow* window)
    {
#ifndef DISABLE_IMGUI
    ImGui_ImplVulkan_Shutdown();
#endif

    return C_STATUS::C_STATUS_OK;
}

} //namespace Cyclone::Render
