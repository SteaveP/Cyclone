#include "ImGuiPlatformWin.h"
#include "PlatformWin.h"

#include "Common/CommonWin.h"
#include "Window/WindowWin.h"

#include "Engine/Platform/Windows/Window/WindowWin.h"
#include "Engine/UI/ImGui/ImGUISubsystem.h"

#include "Engine/UI/ImGui/CommonImGui.h"
#include "Thirdparty/ImGui/backends/imgui_impl_win32.h"

#define ENABLE_IMGUI_IMPL_WIN 1

// forward declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Cyclone
{

C_STATUS ImGUIPlatformWin::OnInit(void* Instance, IUISubsystem* UIModule, IPlatform* Platform, IWindow* Window)
{
    m_Module = UIModule;
    CASSERT(m_Module);

    using namespace std::placeholders;
    Platform->SetOnWindowMessageCallback(std::bind(&ImGUIPlatformWin::OnWindowMessage, this, _1, _2));

#if ENABLE_IMGUI_IMPL_WIN
    ImGuiContext* Context = reinterpret_cast<ImGuiContext*>(Instance);
    ImGui::SetCurrentContext(Context);

    bool Result = ImGui_ImplWin32_Init(Window->GetPlatformWindowHandle());
    C_ASSERT_RETURN_VAL(Result, C_STATUS::C_STATUS_ERROR);
#endif
    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIPlatformWin::OnFrame(void* Instance)
{
#if ENABLE_IMGUI_IMPL_WIN
    ImGui_ImplWin32_NewFrame();
#endif
    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIPlatformWin::OnRender(void* Instance)
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIPlatformWin::OnWindowMessage(void* Instance, void* DataPtr)
{
    CASSERT(DataPtr);

#if ENABLE_IMGUI_IMPL_WIN

    WindowMessageParamWin* Params = reinterpret_cast<WindowMessageParamWin*>(DataPtr);

    LPARAM Result = ImGui_ImplWin32_WndProcHandler(
        Params->hWnd, Params->Message, Params->wParam, Params->lParam);

    return Result ? C_STATUS::C_STATUS_OK : C_STATUS::C_STATUS_INVALID_ARG;
#else
    return C_STATUS::C_STATUS_OK;
#endif
}

C_STATUS ImGUIPlatformWin::OnShutdown(void* Instance, IWindow* window)
{
#if ENABLE_IMGUI_IMPL_WIN
    ImGui_ImplWin32_Shutdown();
#endif
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
