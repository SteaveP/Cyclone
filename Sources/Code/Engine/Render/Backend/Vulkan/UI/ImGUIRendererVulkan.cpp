#include "ImGUIRendererVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/UI/ImGui/CommonImGui.h"
#include "Engine/Core/Profiling.h"

#include "CommonVulkan.h"
#include "RenderBackendVulkan.h"
#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"

#include <backends/imgui_impl_vulkan.h>

#define ENABLE_IMGUI_IMPL_VULKAN 1

namespace Cyclone::Render
{

class ImGUIRendererVulkan::Pimpl
{
public:
    RenderBackendVulkan* Backend;
    Vector<CRenderTargetSet> RenderTargetSets;
};

ImGUIRendererVulkan::ImGUIRendererVulkan() = default;
ImGUIRendererVulkan::~ImGUIRendererVulkan() = default;

C_STATUS ImGUIRendererVulkan::OnInit(void* Instance, IUISubsystem* UISubsystem, Render::IRendererBackend* IBackend, IWindow* Window)
{
    m_pimpl = MakeUnique<Pimpl>();

    m_UISubsystem = UISubsystem;
    m_Window = Window;

    m_pimpl->Backend = dynamic_cast<RenderBackendVulkan*>(IBackend);
    C_ASSERT_RETURN_VAL(m_pimpl->Backend, C_STATUS::C_STATUS_INVALID_ARG);

    // #todo_vk we should be able to render ImGUI with any render pass

    RenderBackendVulkan* Backend = m_pimpl->Backend;
    WindowContextVulkan* WindowContextVk = BACKEND_DOWNCAST(Backend->GetRenderer()->GetWindowContext(m_Window), WindowContextVulkan);

    m_pimpl->RenderTargetSets.resize(WindowContextVk->GetSwapchainImageViewCount());

    for (uint32_t i = 0; i < m_pimpl->RenderTargetSets.size(); ++i)
    {
        auto& RenderTargetSet = m_pimpl->RenderTargetSets[i];
        RenderTargetSet.RenderTargetsCount = 1;
        RenderTargetSet.RenderTargets[0].RenderTarget = WindowContextVk->GetBackBuffer(i);
        RenderTargetSet.RenderTargets[0].InitialLayout = EImageLayoutType::ColorAttachment;
        RenderTargetSet.RenderTargets[0].Layout = EImageLayoutType::ColorAttachment;
        RenderTargetSet.RenderTargets[0].FinalLayout = EImageLayoutType::Present;
        RenderTargetSet.RenderTargets[0].LoadOp = ERenderTargetLoadOp::Load;
        RenderTargetSet.RenderTargets[0].StoreOp = ERenderTargetStoreOp::Store;
    }

#if ENABLE_IMGUI_IMPL_VULKAN
    ImGuiContext* Context = reinterpret_cast<ImGuiContext*>(Instance);
    ImGui::SetCurrentContext(Context);

    ImGui_ImplVulkan_InitInfo InitInfo{};
    InitInfo.Instance = Backend->GetGlobalContext().GetInstance();
    InitInfo.PhysicalDevice = WindowContextVk->GetPhysDevice();
    InitInfo.Device = WindowContextVk->GetLogicDevice();
    InitInfo.Queue = WindowContextVk->GetCommandQueueVk(CommandQueueType::Graphics)->Get();
    InitInfo.QueueFamily = WindowContextVk->GetCommandQueueVk(CommandQueueType::Graphics)->GetQueueFamilyIndex();
    InitInfo.PipelineCache = VK_NULL_HANDLE;
    InitInfo.DescriptorPool = Backend->GetResourceManager(WindowContextVk->GetDeviceHandle())->GetDescriptorPool();
    InitInfo.Subpass = 0;
    InitInfo.MinImageCount = WindowContextVk->GetMinSwapchainImageCount();
    InitInfo.ImageCount = (uint32_t)WindowContextVk->GetSwapchainImageViewCount();
    InitInfo.MSAASamples = WindowContextVk->GetCurrentMsaaSamples();

    CRenderPass RenderPass{};
    RenderPass.RenderTargetSet = m_pimpl->RenderTargetSets[0];
    RenderPass.ViewportExtent = { 0, 0, (float)WindowContextVk->GetSwapchainExtent().width, (float)WindowContextVk->GetSwapchainExtent().height };

    RenderPassVk* RenderPassVkPtr = Backend->GetGlobalContext().GetLogicalDevice(WindowContextVk->GetDeviceHandle()).
        ResourceManager->GetRenderPassVk(RenderPass);

    bool Result = ImGui_ImplVulkan_Init(&InitInfo, RenderPassVkPtr->Get());
    C_ASSERT_RETURN_VAL(Result, C_STATUS::C_STATUS_ERROR);

    {
        CommandQueueVulkan* CommandQueue = WindowContextVk->GetCommandQueueVk(CommandQueueType::Graphics);
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

C_STATUS ImGUIRendererVulkan::OnRender(void* Instance, CCommandBuffer* CommandBuffer)
{
#if ENABLE_IMGUI_IMPL_VULKAN
    // #todo_vk check that current render pass is compatible with those we used in Init method
    CommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CommandBufferVulkan);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBufferVk->Get());
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIRendererVulkan::OnShutdown(void* Instance, IWindow* Window)
{
#if ENABLE_IMGUI_IMPL_VULKAN
    ImGui_ImplVulkan_Shutdown();
#endif

    m_pimpl.reset();

    return C_STATUS::C_STATUS_OK;
}

} //namespace Cyclone::Render
