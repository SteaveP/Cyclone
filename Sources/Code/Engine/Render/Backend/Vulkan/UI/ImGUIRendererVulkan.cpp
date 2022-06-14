#include "ImGUIRendererVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/UI/ImGui/CommonImGui.h"
#include "Engine/Core/Profiling.h"

#include "CommonVulkan.h"
#include "RenderBackendVulkan.h"
#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"
#include "Internal/RenderPassVk.h"
#include "Internal/FrameBufferVk.h"

#include <backends/imgui_impl_vulkan.h>

#define ENABLE_IMGUI_IMPL_VULKAN 1

namespace Cyclone::Render
{

class ImGUIRendererVulkan::Pimpl
{
public:
    RenderBackendVulkan* Backend;
    RenderPassVk RenderPassUI;
    Vector<FrameBufferVk> FrameBufferUI;
};

ImGUIRendererVulkan::ImGUIRendererVulkan() = default;
ImGUIRendererVulkan::~ImGUIRendererVulkan() = default;

C_STATUS ImGUIRendererVulkan::OnInit(void* Instance, IUIModule* UIModule, Render::IRendererBackend* IBackend, IWindow* Window)
{
    m_pimpl = std::make_unique<Pimpl>();

    m_Module = UIModule;
    m_Window = Window;

    m_pimpl->Backend = dynamic_cast<RenderBackendVulkan*>(IBackend);
    C_ASSERT_RETURN_VAL(m_pimpl->Backend, C_STATUS::C_STATUS_INVALID_ARG);

    RenderBackendVulkan* Backend = m_pimpl->Backend;
    WindowContextVulkan* WindowContext = static_cast<WindowContextVulkan*>(Backend->GetRenderer()->GetWindowContext(m_Window));
    {
        Ptr<RenderPassVkInitInfo> RPInitInfo = std::make_shared<RenderPassVkInitInfo>();

        VkFormat ColorFormat = WindowContext->GetSwapchainImageFormat();

        RPInitInfo->ColorAttachmentCount = 1;
        VkAttachmentDescription& ColorAttachment = RPInitInfo->ColorAttachment[0];
        ColorAttachment.format = ColorFormat;
        ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        RPInitInfo->ColorAttachmentRefCount = 1;
        VkAttachmentReference& ColorAttachmentRef = RPInitInfo->ColorAttachmentRef[0];
        ColorAttachmentRef.attachment = 0;
        ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        RPInitInfo->SubpassCount = 1;
        VkSubpassDescription& Subpass = RPInitInfo->Subpass[0];
        Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        Subpass.colorAttachmentCount = 1;
        Subpass.pColorAttachments = &ColorAttachmentRef;

        RPInitInfo->SubpassDependencyCount = 1;
        VkSubpassDependency& SubpassDependency = RPInitInfo->SubpassDependency[0];
        SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        SubpassDependency.dstSubpass = 0;
        SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.srcAccessMask = 0;
        SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        RPInitInfo->Backend = Backend;
        RPInitInfo->Device = WindowContext->GetDevice();

        C_STATUS Result = m_pimpl->RenderPassUI.Init(RPInitInfo);
        CASSERT(C_SUCCEEDED(Result));
    }

    {
        FrameBufferVkInitInfo FBInitInfo{};
        FBInitInfo.Backend = Backend;
        FBInitInfo.Device = WindowContext->GetDevice();
        FBInitInfo.RenderPass = &m_pimpl->RenderPassUI;
        FBInitInfo.Width = WindowContext->GetSwapchainExtent().width;
        FBInitInfo.Height = WindowContext->GetSwapchainExtent().height;
        FBInitInfo.Layers = 1;

        m_pimpl->FrameBufferUI.resize(WindowContext->GetSwapchainImageViewCount());
        for (uint32_t i = 0; i < m_pimpl->FrameBufferUI.size(); ++i)
        {
            FBInitInfo.Attachments[0] = WindowContext->GetSwapchainImageView(i);
            FBInitInfo.AttachmentsCount = 1;

            C_STATUS Result = m_pimpl->FrameBufferUI[i].Init(FBInitInfo);
            CASSERT(C_SUCCEEDED(Result));
        }
    }

#if ENABLE_IMGUI_IMPL_VULKAN
    ImGuiContext* Context = reinterpret_cast<ImGuiContext*>(Instance);
    ImGui::SetCurrentContext(Context);

    ImGui_ImplVulkan_InitInfo InitInfo{};
    InitInfo.Instance = Backend->GetGlobalContext().GetInstance();
    InitInfo.PhysicalDevice = WindowContext->GetPhysDevice();
    InitInfo.Device = WindowContext->GetLogicDevice();
    InitInfo.Queue = WindowContext->GetCommandQueueVk(CommandQueueType::Graphics)->Get();
    InitInfo.QueueFamily = WindowContext->GetCommandQueueVk(CommandQueueType::Graphics)->GetQueueFamilyIndex();
    InitInfo.PipelineCache = VK_NULL_HANDLE;
    InitInfo.DescriptorPool = Backend->GetDescriptorPool();
    InitInfo.Subpass = 0;
    InitInfo.MinImageCount = WindowContext->GetMinSwapchainImageCount();
    InitInfo.ImageCount = (uint32_t)WindowContext->GetSwapchainImageViewCount();
    InitInfo.MSAASamples = WindowContext->GetCurrentMsaaSamples();

    bool Result = ImGui_ImplVulkan_Init(&InitInfo, m_pimpl->RenderPassUI.Get());
    C_ASSERT_RETURN_VAL(Result, C_STATUS::C_STATUS_ERROR);

    {
        CommandQueueVulkan* CommandQueue = WindowContext->GetCommandQueueVk(CommandQueueType::Graphics);
        CASSERT(CommandQueue);

        CommandBufferVulkan* CommandBuffer = CommandQueue->AllocateCommandBuffer();
        CASSERT(CommandBuffer);

        CommandBuffer->Begin();

        ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer->Get());

        CommandBuffer->End();

        C_STATUS Res = CommandQueue->SubmitVk(&CommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, true);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Res), Res);

        // wait for the completion
        VkResult ResultVk = vkQueueWaitIdle(CommandQueue->Get());
        C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);
    }
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIRendererVulkan::OnFrame(void* Instance)
{
#if ENABLE_IMGUI_IMPL_VULKAN
    ImGui_ImplVulkan_NewFrame();
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIRendererVulkan::OnRender(void* Instance)
{
#if ENABLE_IMGUI_IMPL_VULKAN
    WindowContextVulkan* WindowContext = static_cast<WindowContextVulkan*>(m_pimpl->Backend->GetRenderer()->GetWindowContext(m_Window));

    CommandQueueVulkan* CommandQueue = WindowContext->GetCommandQueueVk(CommandQueueType::Graphics);
    CASSERT(CommandQueue);

    CommandBufferVulkan* CommandBuffer = CommandQueue->AllocateCommandBuffer();
    CASSERT(CommandBuffer);

    PROFILE_GPU_SCOPED_EVENT(CommandBuffer->Get(), "ImGUI Render");

    CommandBuffer->Begin();

    // #todo_vk render pass must be setted up from external env as well as command list already should be prepared with correct state
    {
        RenderPassVkBeginInfo RPBeginInfo{};
        RPBeginInfo.FrameBuffer = &m_pimpl->FrameBufferUI[WindowContext->GetCurrentImageIndex()];
        RPBeginInfo.RenderArea.offset = { 0, 0 };
        RPBeginInfo.RenderArea.extent = WindowContext->GetSwapchainExtent();

        m_pimpl->RenderPassUI.Begin(CommandBuffer, RPBeginInfo);
    }

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer->Get());

    m_pimpl->RenderPassUI.End(CommandBuffer);

    CommandBuffer->End();

    CommandQueue->SubmitVk(&CommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, true);
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIRendererVulkan::OnShutdown(void* Instance, IWindow* Window)
{
#if ENABLE_IMGUI_IMPL_VULKAN
    ImGui_ImplVulkan_Shutdown();
#endif

    return C_STATUS::C_STATUS_OK;
}

} //namespace Cyclone::Render
