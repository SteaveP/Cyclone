#include "ImGUIRendererVulkan.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/Utils/Profiling.h"

#include "Engine/UI/ImGui/CommonImGui.h"

#include "Engine/Render/Backend/Resource.h"

#include "CommonVulkan.h"
#include "ResourceViewVulkan.h"
#include "RenderBackendVulkan.h"
#include "CommandQueueVulkan.h"
#include "CommandBufferVulkan.h"
#include "WindowContextVulkan.h"

#include "ResourceManagerVulkan.h"

// use local modified copy
#include "imgui_impl_vulkan.h"

#define ENABLE_IMGUI_IMPL_VULKAN 1

namespace Cyclone::Render
{

class CImGUIRendererVulkan::Pimpl
{
public:
    CRenderBackendVulkan* BackendVk = nullptr;
    CHandle<CSampler> Sampler;
    UniquePtr<CDescriptorPoolVk> DescriptorPool;
};

CImGUIRendererVulkan::CImGUIRendererVulkan() = default;
CImGUIRendererVulkan::~CImGUIRendererVulkan() = default;

C_STATUS CImGUIRendererVulkan::OnInit(void* Instance, IUISubsystem* UISubsystem, IRendererBackend* IBackend, IWindow* Window)
{
    m_pimpl = MakeUnique<Pimpl>();

    m_UISubsystem = UISubsystem;
    m_Window = Window;

    m_pimpl->BackendVk = GET_BACKEND_IMPL(IBackend);
    C_ASSERT_RETURN_VAL(m_pimpl->BackendVk, C_STATUS::C_STATUS_INVALID_ARG);

    CRenderBackendVulkan* BackendVk = m_pimpl->BackendVk;
    CWindowContextVulkan* WindowContextVk = BACKEND_DOWNCAST(BackendVk->GetRenderer()->GetWindowContext(m_Window), CWindowContextVulkan);

    auto& Device = BackendVk->GetDeviceManager().GetDevice(WindowContextVk->GetDeviceHandle());

    EFormatType BackBufferFormat = EFormatType::Undefined;
    {
        CASSERT(WindowContextVk->GetBackBuffersCount() > 0);
        if (CResource* BackBufferPtr = Device.ResourceManager->GetResource(WindowContextVk->GetBackBuffer(0).Texture))
        {
            BackBufferFormat = BackBufferPtr->GetDesc().Format;
        }

        CASSERT(BackBufferFormat != EFormatType::Undefined);
    }

    {
        CSamplerDesc SamplerDesc{};
        SamplerDesc.Backend = BackendVk;
        SamplerDesc.DeviceHandle = WindowContextVk->GetDeviceHandle();
        m_pimpl->Sampler = BackendVk->GetResourceManagerVk(WindowContextVk->GetDeviceHandle())->GetSamplerCached(SamplerDesc);
    }

    m_pimpl->DescriptorPool = MakeUnique<CDescriptorPoolVk>();
    {
        CDescriptorPoolVkDesc Desc{};
        Desc.DescriptorsCount = 300;
        Desc.DeviceHandle = WindowContextVk->GetDeviceHandle();
        Desc.BackendVk = BackendVk;
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "DescriptorPool ImGUI";
#endif

        C_STATUS Result = m_pimpl->DescriptorPool->Init(Desc);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

#if ENABLE_IMGUI_IMPL_VULKAN
    ImGuiContext* Context = reinterpret_cast<ImGuiContext*>(Instance);
    ImGui::SetCurrentContext(Context);

    ImGui_ImplVulkan_InitInfo InitInfo{};
    InitInfo.Instance = BackendVk->GetDeviceManager().GetInstanceVk();
    InitInfo.PhysicalDevice = BackendVk->GetDeviceManager().GetPhysDevice(WindowContextVk->GetDeviceHandle()).DeviceVk;
    InitInfo.Device = Device.DeviceVk;
    InitInfo.Queue = WindowContextVk->GetCommandQueueVk(CommandQueueType::Graphics)->Get();
    InitInfo.QueueFamily = WindowContextVk->GetCommandQueueVk(CommandQueueType::Graphics)->GetQueueFamilyIndex();
    InitInfo.PipelineCache = VK_NULL_HANDLE;
    InitInfo.DescriptorPool = m_pimpl->DescriptorPool->Get();
    InitInfo.Subpass = 0;
    InitInfo.MinImageCount = WindowContextVk->GetMinSwapchainImageCount();
    InitInfo.ImageCount = (uint32_t)WindowContextVk->GetBackBuffersCount();
    InitInfo.MSAASamples = WindowContextVk->GetCurrentMsaaSamples();


    bool Result = true;

#if ENABLE_VOLK_LOADER
    Result = ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* Data) {
        return vkGetInstanceProcAddr(VkInstance(Data), function_name);
    }, InitInfo.Instance);
    C_ASSERT_RETURN_VAL(Result, C_STATUS::C_STATUS_ERROR);
#endif

    Result = ImGui_ImplVulkan_Init(&InitInfo, ConvertFormatType(BackBufferFormat));
    C_ASSERT_RETURN_VAL(Result, C_STATUS::C_STATUS_ERROR);

    {
        CCommandQueueVulkan* CommandQueue = WindowContextVk->GetCommandQueueVk(CommandQueueType::Graphics);
        CASSERT(CommandQueue);

        CCommandBufferVulkan* CommandBuffer = CommandQueue->AllocateCommandBufferVk();
        CASSERT(CommandBuffer);

        CommandBuffer->Begin();

        ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer->Get());

        CommandBuffer->End();

        C_STATUS Res = CommandQueue->SubmitVk(&CommandBuffer, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, true);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Res), Res);

        // wait for the completion
        VkResult ResultVk = VK_CALL(Device, vkQueueWaitIdle(CommandQueue->Get()));
        C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);
    }
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CImGUIRendererVulkan::OnFrame(void* Instance)
{
#if ENABLE_IMGUI_IMPL_VULKAN
    ImGui_ImplVulkan_NewFrame();
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CImGUIRendererVulkan::OnRender(void* Instance, CCommandBuffer* CommandBuffer)
{
#if ENABLE_IMGUI_IMPL_VULKAN
    // #todo_vk check that current render pass is compatible with those we used in Init method
    CCommandBufferVulkan* CommandBufferVk = BACKEND_DOWNCAST(CommandBuffer, CCommandBufferVulkan);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBufferVk->Get());
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CImGUIRendererVulkan::OnShutdown(void* Instance, IWindow* Window)
{

#if ENABLE_IMGUI_IMPL_VULKAN
    ImGui_ImplVulkan_Shutdown();
#endif

    if (m_pimpl && m_pimpl->BackendVk)
    {
        CRenderBackendVulkan* BackendVk = m_pimpl->BackendVk;
        CWindowContextVulkan* WindowContextVk = BACKEND_DOWNCAST(BackendVk->GetRenderer()->GetWindowContext(m_Window), CWindowContextVulkan);

        auto& Device = BackendVk->GetDeviceManager().GetDevice(WindowContextVk->GetDeviceHandle());

        if (m_pimpl->Sampler.IsValid())
            Device.ResourceManager->ReleaseSamplerCached(m_pimpl->Sampler);
    }
    m_pimpl.reset();

    return C_STATUS::C_STATUS_OK;
}

RawPtr CImGUIRendererVulkan::RegisterTexture(CHandle<CResourceView> View, EImageLayoutType ExpectedLayout)
{
    CResourceManagerVulkan* ResourceManagerVk = m_pimpl->BackendVk->GetResourceManagerVk(CDeviceHandle::From(View));
    CASSERT(ResourceManagerVk);

    CResourceViewVulkan* ViewPtr = BACKEND_DOWNCAST(ResourceManagerVk->GetResourceView(View), CResourceViewVulkan);
    CASSERT(ViewPtr);

    CSamplerVulkan* SamplerPtr = BACKEND_DOWNCAST(ResourceManagerVk->GetSampler(m_pimpl->Sampler), CSamplerVulkan);
    CASSERT(SamplerPtr);

    return ImGui_ImplVulkan_AddTexture(SamplerPtr->Get(), ViewPtr->GetTextureView(),
        ConvertLayoutType(ExpectedLayout));
}

void CImGUIRendererVulkan::UnRegisterTexture(CHandle<CResourceView> View, RawPtr Descriptor)
{
    m_pimpl->BackendVk->GetDisposalManagerVk(CDeviceHandle::From(View))->AddDisposable([Descriptor]()
    {
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)Descriptor);
    });
}

} //namespace Cyclone::Render
