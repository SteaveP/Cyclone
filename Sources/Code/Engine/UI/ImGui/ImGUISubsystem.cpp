#include "ImGUISubsystem.h"

#include "Engine/UI/ImGui/CommonImGui.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IPlatform.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IWindow.h"

#include "Engine/UI/ImGui/ImGuiRenderer.h"
#include "Engine/UI/ImGui/ImGuiPlatform.h"

#include "Engine/Utils/Profiling.h"
#include "Engine/Utils/Config.h"

#define C_IMGUI_MODULE_DEFAULT_DPI 96

#define GET_CONFIG_UI() GET_CONFIG()["UI"]

namespace Cyclone
{

ImGUISubsystem::ImGUISubsystem() = default;
ImGUISubsystem::ImGUISubsystem(ImGUISubsystem&& Other) noexcept : IUISubsystem(MoveTemp(Other))
{
    std::swap(m_App, Other.m_App);
    std::swap(m_Context, Other.m_Context);
    std::swap(m_Platform, Other.m_Platform);
    std::swap(m_Renderer, Other.m_Renderer);
}
ImGUISubsystem& ImGUISubsystem::operator=(ImGUISubsystem&& Other) noexcept
{
    if (this != &Other)
    {
        IUISubsystem::operator=(MoveTemp(Other));
        std::swap(m_App, Other.m_App);
        std::swap(m_Context, Other.m_Context);
        std::swap(m_Platform, Other.m_Platform);
        std::swap(m_Renderer, Other.m_Renderer);
    }
    return *this;
}
ImGUISubsystem::~ImGUISubsystem()
{
    ShutdownImpl();
}

C_STATUS ImGUISubsystem::Init(IApplication* App, float Dpi)
{
    m_App = App;

    C_ASSERT_RETURN_VAL(m_Renderer, C_STATUS::C_STATUS_INVALID_ARG);
    C_ASSERT_RETURN_VAL(m_Platform, C_STATUS::C_STATUS_INVALID_ARG);

    IMGUI_CHECKVERSION();

    bool ViewportsEnabled = true;
    ViewportsEnabled = GET_CONFIG_UI().value("EnableViewports", ViewportsEnabled);
    bool DarkThemeEnabled = true;
    DarkThemeEnabled = GET_CONFIG_UI().value("EnableDarkTheme", DarkThemeEnabled);
    // #todo_ui #todo_imgui ImGui_ImplWin32_EnableAlphaCompositing

    m_Context = ImGui::CreateContext();
    C_ASSERT_RETURN_VAL(m_Context, C_STATUS::C_STATUS_ERROR);

    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    if (ViewportsEnabled)
    {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }

    if (DarkThemeEnabled)
    {
        ImGui::StyleColorsDark();
    }
    else
    {
        ImGui::StyleColorsLight();
    }

    // Platform and Renderer init
    // #todo_ui make support for multiple app windows (not related to viewports feature)
    CASSERT(m_App->GetWindowsCount() >= 1);

    uint32 WindowIndex = 0;
    C_STATUS Result = m_Platform->OnInit(m_Context, this, m_App->GetPlatform(), m_App->GetWindow(WindowIndex));
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = m_Renderer->OnInit(m_Context, this, m_App->GetRenderer()->GetRendererBackend(), m_App->GetWindow(WindowIndex));
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    using namespace std::placeholders;
    m_App->GetPlatform()->SetOnDPIChangedCallback(std::bind(&ImGUISubsystem::OnDPIChanged, this, _1, _2));

    // Set DPI
    ImGUISubsystem::OnDPIChanged(Dpi, C_IMGUI_MODULE_DEFAULT_DPI);

    return C_STATUS::C_STATUS_OK;
}

void ImGUISubsystem::Shutdown() noexcept
{
    ShutdownImpl();
}

void ImGUISubsystem::ShutdownImpl() noexcept
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

C_STATUS ImGUISubsystem::OnRender(Render::CCommandBuffer* CommandBuffer)
{
    PROFILE_CPU_SCOPED_EVENT("ImGUI OnRender");
    ImGui::Render();

    {
        C_STATUS Result = m_Platform->OnRender(m_Context);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

        Result = m_Renderer->OnRender(m_Context, CommandBuffer);
        C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUISubsystem::OnEndFrame()
{
    // Update and Render additional Platform Windows
    auto& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
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

RawPtr ImGUISubsystem::RegisterTexture(Render::CHandle<Render::CResourceView> View, Render::EImageLayoutType ExpectedLayout)
{
    return m_Renderer->RegisterTexture(View, ExpectedLayout);
}

void ImGUISubsystem::UnRegisterTexture(Render::CHandle<Render::CResourceView> View, RawPtr Descriptor)
{
    m_Renderer->UnRegisterTexture(View, Descriptor);
}

} // namespace Cyclone
