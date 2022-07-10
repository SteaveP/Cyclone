#include "ImGUIRendererDX12.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/UI/ImGui/CommonImGui.h"

#include "CommonDX12.h"
#include "RenderBackendDX12.h"

#include <backends/imgui_impl_dx12.h>

#define ENABLE_IMGUI_IMPL_DX12 0

namespace Cyclone::Render
{

class ImGUIRendererDX12::Pimpl
{
public:
    RenderBackendDX12* Backend;
};

ImGUIRendererDX12::ImGUIRendererDX12() = default;
ImGUIRendererDX12::~ImGUIRendererDX12() = default;

C_STATUS ImGUIRendererDX12::OnInit(void* Instance, IUISubsystem* UISubsystem, Render::IRendererBackend* IBackend, IWindow* Window)
{
    m_pimpl = MakeUnique<Pimpl>();

    m_UISubsystem = UISubsystem;
    m_Window = Window;

    m_pimpl->BackendDX12 = GET_BACKEND_IMPL(IBackend);
    C_ASSERT_RETURN_VAL(m_pimpl->BackendDX12, C_STATUS::C_STATUS_INVALID_ARG);

    RenderBackendDX12* Backend = m_pimpl->BackendDX12;
    WindowContextDX12* WindowContext = static_cast<WindowContextDX12*>(Backend->GetRenderer()->GetWindowContext(m_Window));
 
    CASSERT(false); // #todo_dx12 Implement it first!

#if ENABLE_IMGUI
    ImGuiContext* Context = reinterpret_cast<ImGuiContext*>(Instance);
    ImGui::SetCurrentContext(Context);

    bool Result = ImGui_ImplDX12_Init(&InitInfo, m_pimpl->RenderPassUI.Get());
    C_ASSERT_RETURN_VAL(Result, C_STATUS::C_STATUS_ERROR);
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIRendererDX12::OnFrame(void* Instance)
{
#if ENABLE_IMGUI
    ImGui_ImplDX12_NewFrame();
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIRendererDX12::OnRender(void* Instance)
{
#if ENABLE_IMGUI

    RenderBackendDX12* Backend = m_pimpl->BackendDX12;
    WindowContextDX12* WindowContext = static_cast<WindowContextDX12*>(Backend->GetRenderer()->GetWindowContext(m_Window));

    CommandQueueDX12* CommandQueue = WindowContext->GetCommandQueueVk(CommandQueueType::Graphics);
    CASSERT(CommandQueue);

    CommandBufferDX12* CommandBuffer = CommandQueue->AllocateCommandBuffer();
    CASSERT(CommandBuffer);

    CommandBuffer->Begin();
    
    {
        ID3D12CommandList* CommandList = nullptr;
        PROFILE_GPU_SCOPED_EVENT(CommandList, "ImGUI Render");

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CommandList);
    }

    CommandBuffer->End();

    CommandQueue->Submit(&CommandBuffer, 1, true);
#endif

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIRendererDX12::OnShutdown(void* Instance, IWindow* Window)
{
#if ENABLE_IMGUI
    ImGui_ImplDX12_Shutdown();
#endif

    m_pimpl.reset();

    return C_STATUS::C_STATUS_OK;
}

} //namespace Cyclone::Render
