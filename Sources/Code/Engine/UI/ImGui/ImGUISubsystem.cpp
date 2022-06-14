#include "ImGUISubsystem.h"

#include "Engine/UI/ImGui/CommonImGui.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IPlatform.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IWindow.h"

#include "Engine/UI/ImGui/ImGuiRenderer.h"
#include "Engine/UI/ImGui/ImGuiPlatform.h"

#include "Engine/Core/Profiling.h"

#define C_IMGUI_MODULE_DEFAULT_DPI 96

namespace Cyclone
{

C_STATUS ImGUISubsystem::Init(IApplication* App, float Dpi)
{
    m_App = App;

    C_ASSERT_RETURN_VAL(m_Renderer, C_STATUS::C_STATUS_INVALID_ARG);
    C_ASSERT_RETURN_VAL(m_Platform, C_STATUS::C_STATUS_INVALID_ARG);

    IMGUI_CHECKVERSION();

    m_Context = ImGui::CreateContext();
    C_ASSERT_RETURN_VAL(m_Context, C_STATUS::C_STATUS_ERROR);

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // platform and renderer init
    C_STATUS Result = m_Platform->OnInit(m_Context, this, m_App->GetPlatform(), m_App->GetWindow(0));
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = m_Renderer->OnInit(m_Context, this, m_App->GetRenderer()->GetRendererBackend(), m_App->GetWindow(0));
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    using namespace std::placeholders;
    m_App->GetPlatform()->SetOnDPIChangedCallback(std::bind(&ImGUISubsystem::OnDPIChanged, this, _1, _2));

    // Set DPI
    ImGUISubsystem::OnDPIChanged(Dpi, C_IMGUI_MODULE_DEFAULT_DPI);

    return C_STATUS::C_STATUS_OK;
}

void ImGUISubsystem::Shutdown() noexcept
{
    C_STATUS Result = m_Renderer->OnShutdown(m_Context, m_App->GetWindow(0));
    C_ASSERT_RETURN(C_SUCCEEDED(Result));

    Result = m_Platform->OnShutdown(m_Context, m_App->GetWindow(0));
    C_ASSERT_RETURN(C_SUCCEEDED(Result));

    ImGui::DestroyContext(m_Context);
    m_Context = nullptr;
}

C_STATUS ImGUISubsystem::OnFrame()
{
    C_STATUS Result = m_Renderer->OnFrame(m_Context);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = m_Platform->OnFrame(m_Context);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    ImGui::NewFrame();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUISubsystem::OnRender()
{
    PROFILE_CPU_SCOPED_EVENT("ImGUI OnRender");
    ImGui::Render();

    C_STATUS Result = m_Platform->OnRender(m_Context);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = m_Renderer->OnRender(m_Context);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUISubsystem::OnWindowMessage(void* Params)
{
    C_STATUS Result = m_Platform->OnWindowMessage(m_Context, Params);
    return Result;
}

C_STATUS ImGUISubsystem::OnDPIChanged(float NewDPI, float OldDPI)
{
    float ScaleFactorFromDefault = NewDPI / C_IMGUI_MODULE_DEFAULT_DPI;

    // This is an old and obsolete API!
    // For correct scaling, prefer to reload font + rebuild ImFontAtlas + call style.ScaleAllSizes().
    auto& io = ImGui::GetIO();
    io.FontGlobalScale = ScaleFactorFromDefault;

    float ScaleFactorFromPrevious = NewDPI / OldDPI;
    ImGui::GetStyle().ScaleAllSizes(ScaleFactorFromPrevious);

    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
