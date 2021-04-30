#include "ImGuiPlatformWin.h"
#include "../PlatformWin.h"

#include "../Common/CommonWin.h"
#include "../Window/WindowWin.h"

#include "Engine/Platform/Windows/Window/WindowWin.h"
#include "Engine/UI/ImGui/ImGuiModule.h"

#include "Engine/UI/ImGui/CommonImGui.h"
#include "Thirdparty/ImGui/backends/imgui_impl_win32.h"

//#define DISABLE_IMGUI

// forward declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Cyclone
{

C_STATUS ImGUIPlatformWin::OnInit(void* Instance, IUIModule* UIModule, IPlatform* Platform, IWindow* window)
{
    m_module = UIModule;
    CASSERT(m_module);

    using namespace std::placeholders;
    Platform->SetOnWindowMessageCallback(std::bind(&ImGUIPlatformWin::OnWindowMessage, this, _1, _2));

#ifndef DISABLE_IMGUI
    ImGuiContext* Context = reinterpret_cast<ImGuiContext*>(Instance);
    ImGui::SetCurrentContext(Context);

    bool result = ImGui_ImplWin32_Init(window->GetPlatformWindowHandle());
    C_ASSERT_RETURN_VAL(result, C_STATUS::C_STATUS_ERROR);
#endif
    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIPlatformWin::OnFrame(void* Instance)
{
#ifndef DISABLE_IMGUI
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

#ifndef DISABLE_IMGUI

    WindowMessageParamWin* Params = reinterpret_cast<WindowMessageParamWin*>(DataPtr);

    LPARAM result = ImGui_ImplWin32_WndProcHandler(
        Params->hWnd, Params->message, Params->wParam, Params->lParam);

    return result ? C_STATUS::C_STATUS_OK : C_STATUS::C_STATUS_INVALID_ARG;
#else
    return C_STATUS::C_STATUS_OK;
#endif
}

C_STATUS ImGUIPlatformWin::OnShutdown(void* Instance, IWindow* window)
{
#ifndef DISABLE_IMGUI
    ImGui_ImplWin32_Shutdown();
#endif
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
