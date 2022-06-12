#include "ImGuiModule.h"

#include "Engine/UI/ImGui/CommonImGui.h"

#include "Engine/Framework/IApplication.h"
#include "Engine/Framework/IPlatform.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IWindow.h"

#include "Engine/UI/ImGui/ImGuiRenderer.h"
#include "Engine/UI/ImGui/ImGuiPlatform.h"

#define C_IMGUI_MODULE_DEFAULT_DPI 96

namespace Cyclone
{

C_STATUS ImGUIModule::Init(IApplication* app, float dpi)
{
    m_app = app;

    C_ASSERT_RETURN_VAL(m_renderer, C_STATUS::C_STATUS_INVALID_ARG);
    C_ASSERT_RETURN_VAL(m_platform, C_STATUS::C_STATUS_INVALID_ARG);

    IMGUI_CHECKVERSION();

    m_context = ImGui::CreateContext();
    C_ASSERT_RETURN_VAL(m_context, C_STATUS::C_STATUS_ERROR);

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // platform and renderer init
    C_STATUS result = m_platform->OnInit(m_context, this, m_app->GetPlatform(), m_app->GetWindow());
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    result = m_renderer->OnInit(m_context, this, m_app->GetRenderer()->GetRendererBackend(), m_app->GetWindow());
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(result), result);

    using namespace std::placeholders;
    m_app->GetPlatform()->SetOnDPIChangedCallback(std::bind(&ImGUIModule::OnDPIChanged, this, _1, _2));

    // Set DPI
    ImGUIModule::OnDPIChanged(dpi, C_IMGUI_MODULE_DEFAULT_DPI);

    return C_STATUS::C_STATUS_OK;
}

void ImGUIModule::Shutdown() noexcept
{
    C_STATUS Result = m_renderer->OnShutdown(m_context, m_app->GetWindow());
    C_ASSERT_RETURN(C_SUCCEEDED(Result));

    Result = m_platform->OnShutdown(m_context, m_app->GetWindow());
    C_ASSERT_RETURN(C_SUCCEEDED(Result));

    ImGui::DestroyContext(m_context);
    m_context = nullptr;
}

C_STATUS ImGUIModule::OnFrame()
{
    C_STATUS Result = m_renderer->OnFrame(m_context);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = m_platform->OnFrame(m_context);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    ImGui::NewFrame();

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIModule::OnRender()
{
    //PROFILE_CPU_SCOPED_EVENT("ImGUI OnRender");
    ImGui::Render();

    C_STATUS Result = m_platform->OnRender(m_context);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    Result = m_renderer->OnRender(m_context);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    return C_STATUS::C_STATUS_OK;
}

C_STATUS ImGUIModule::OnWindowMessage(void* params)
{
    C_STATUS Result = m_platform->OnWindowMessage(m_context, params);
    return Result;
}

C_STATUS ImGUIModule::OnDPIChanged(float newDPI, float oldDPI)
{
    float scaleFactorFromDefault = newDPI / C_IMGUI_MODULE_DEFAULT_DPI;

    // This is an old and obsolete API!
    // For correct scaling, prefer to reload font + rebuild ImFontAtlas + call style.ScaleAllSizes().
    auto& io = ImGui::GetIO();
    io.FontGlobalScale = scaleFactorFromDefault;

    float scaleFactorFromPrevious = newDPI / oldDPI;
    ImGui::GetStyle().ScaleAllSizes(scaleFactorFromPrevious);

    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
