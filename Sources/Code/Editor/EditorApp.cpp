#include "EditorApp.h"

#include "Engine/UI/CommonUI.h"
#include "Engine/Framework/IWindow.h"
#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IUISubsystem.h"

#include "Engine/Scene/Camera.h"
#include "Engine/Core/Math.h"

#include "Engine/Render/Backend/IRendererBackend.h"
#include "Engine/Render/Backend/IResourceManager.h"
#include "Engine/Render/Backend/WindowContext.h"
#include "Engine/Render/Backend/Resource.h"

#include "Engine/Scene/SceneSubsystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneViewport.h"

#include "Engine/Render/Scene/RenderSceneSubsystem.h"
#include "Engine/Render/Scene/SceneRenderer.h"
#include "Engine/Render/Scene/RenderSceneView.h"

namespace Cyclone
{

static String ED_SCENE_NAME = "EditorScene";

C_STATUS CEditorApplication::OnInit()
{
    IRenderer* Renderer = GetRenderer();
    CASSERT(Renderer);

    m_SceneSys = (CSceneSubsystem*)GetSubsystem("SceneSubsystem");
    m_RenderSceneSys = (Render::CRenderSceneSubsystem*)GetSubsystem("RenderSceneSubsystem");
    CASSERT(m_SceneSys && m_RenderSceneSys);

    m_RenderSceneSys->OnRenderSceneViewAddedDelegate += DelegateLib::DelegateMember1(this, &CEditorApplication::OnAddRenderSceneView);
    m_RenderSceneSys->OnRenderSceneViewRemovedDelegate += DelegateLib::DelegateMember1(this, &CEditorApplication::OnRemoveRenderSceneView);

    // #todo_editor setup default scene (Lights, Cube, Camera)
    

    CASSERT(m_Windows.size() > 0);
    Ptr<IWindow> Window = m_Windows[0];
    
    Ptr<CScene> Scene = m_SceneSys->AddScene(ED_SCENE_NAME);
    CASSERT(Scene);

    // #todo_editor refactor + read from saved layout/config/command line?
    uint32 ViewportsCount = 2;
    for (uint32 i = 0; i < ViewportsCount; ++i)
    {
        m_SceneSys->AddViewport("Viewport" + ToString(i), Scene, Window);
    }

    return C_STATUS::C_STATUS_OK;
}

CEditorApplication::~CEditorApplication()
{
    DeInitImpl();
}

void CEditorApplication::DeInit()
{
    DeInitImpl();
    CDefaultApplication::DeInit();
}

void CEditorApplication::DeInitImpl()
{
    WaitAllPendingJobs();

    if (m_SceneSys)
    {
        m_SceneSys->RemoveScene(ED_SCENE_NAME);
    }

    m_ViewportRenderTargetsDescriptorSet.clear();
    m_ViewportOpened.clear();

    if (m_RenderSceneSys)
    {
        m_RenderSceneSys->OnRenderSceneViewAddedDelegate -= DelegateLib::DelegateMember1(this, &CEditorApplication::OnAddRenderSceneView);
        m_RenderSceneSys->OnRenderSceneViewRemovedDelegate -= DelegateLib::DelegateMember1(this, &CEditorApplication::OnRemoveRenderSceneView);
    }

    m_SceneSys = nullptr;
    m_RenderSceneSys = nullptr;
}

void CEditorApplication::OnAddRenderSceneView(Ptr<Render::CRenderSceneView> View)
{
    m_ViewportRenderTargetsDescriptorSet.emplace_back(
        m_UI->RegisterTexture(View->RenderTarget.ShaderResourceView, Render::EImageLayoutType::ShaderReadOnly));

    m_ViewportOpened.emplace_back(true);
}

void CEditorApplication::OnRemoveRenderSceneView(Ptr<Render::CRenderSceneView> View)
{
    uint32 Index = ~0u;
    for (uint32 i = 0; i < (uint32)m_SceneSys->GetViewportCount(); ++i)
    {
        if (View->Viewport == m_SceneSys->GetViewport(i).get())
        {
            Index = i;
            break;
        }
    }
    C_ASSERT_RETURN(Index != ~0u);

    m_UI->UnRegisterTexture(View->RenderTarget.ShaderResourceView, m_ViewportRenderTargetsDescriptorSet[Index]);

    m_ViewportRenderTargetsDescriptorSet.erase(m_ViewportRenderTargetsDescriptorSet.begin() + Index);
    m_ViewportOpened.erase(m_ViewportOpened.begin() + Index);
}

C_STATUS CEditorApplication::OnBeginFrame()
{
    // Remove old viewports
    for (uint32 i = 0; i <  m_ViewportOpened.size(); ++i)
    {
        if (m_ViewportOpened[i] == false)
        {
            m_SceneSys->RemoveViewport(m_SceneSys->GetViewport(i));
            --i;
        }
    }

    return C_STATUS::C_STATUS_OK;
}

C_STATUS CEditorApplication::OnUpdate()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CEditorApplication::OnUpdateUI()
{
    if (ImGui::GetCurrentContext() == nullptr)
        return C_STATUS::C_STATUS_OK;

    ImGui::DockSpaceOverViewport(nullptr, 0/*ImGuiDockNodeFlags_PassthruCentralNode*/);

    ShowMenu();

    ShowViewports();
    ShowWorldOutliner();
    ShowProperties();
    ShowContentBrowser();

    return C_STATUS::C_STATUS_OK;
}
void CEditorApplication::ShowMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("(demo menu)", NULL, false, false);
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            if (ImGui::BeginMenu("Open Recent"))
            {
                ImGui::MenuItem("Project1.cyclonep");
                ImGui::MenuItem("Map1.cmap");
                if (ImGui::BeginMenu("More.."))
                {
                    ImGui::MenuItem("Hello");
                    ImGui::MenuItem("Sailor");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save As..")) {}

            ImGui::Separator();

            if (ImGui::BeginMenu("Options"))
            {
                static bool enabled = true;
                ImGui::MenuItem("Enabled", "", &enabled);
                ImGui::BeginChild("child", ImVec2(0, 60), true);
                for (int i = 0; i < 10; i++)
                    ImGui::Text("Scrolling Text %d", i);
                ImGui::EndChild();
                static float f = 0.5f;
                static int n = 0;
                ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
                ImGui::InputFloat("Input", &f, 0.1f);
                ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
                ImGui::EndMenu();
            }

            // Here we demonstrate appending again to the "Options" menu (which we already created above)
            // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
            // In a real code-base using it would make senses to use this feature from very different code locations.
            if (ImGui::BeginMenu("Options")) // <-- Append!
            {
                static bool b = true;
                ImGui::Checkbox("SomeOption", &b);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Disabled", false)) // Disabled
            {
                IM_ASSERT(0);
            }
            if (ImGui::MenuItem("Checked", NULL, true)) {}
            if (ImGui::MenuItem("Quit", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Add Viewport"))
            {
                m_SceneSys->AddViewport("Viewport" + ToString(m_ViewportOpened.size()), m_SceneSys->GetScene(ED_SCENE_NAME), m_Windows[0]);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void CEditorApplication::ShowViewports()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

    CASSERT(m_SceneSys->GetViewportCount() == m_ViewportOpened.size());

    for (uint32 i = 0; i < m_ViewportOpened.size(); ++i)
    {
        Ptr<CSceneViewport> Viewport = m_SceneSys->GetViewport(i);
        String WindowName = Viewport->GetName(); // #todo_editor what if name changed?

        Ptr<Render::CRenderSceneView> ViewportView = m_RenderSceneSys->GetRenderSceneView(Viewport.get());

        Render::IResourceManager* ResourceManager = m_Renderer->GetRendererBackend()->GetResourceManager(ViewportView->RenderWindowContext->GetDeviceHandle());

        bool IsOpened = m_ViewportOpened[i];
        if (ImGui::Begin(WindowName.c_str(), &IsOpened) && IsOpened)
        {
            ImVec2 vMin = ImGui::GetWindowContentRegionMin();
            ImVec2 vMax = ImGui::GetWindowContentRegionMax();
            ImVec2 vSize = ImVec2(vMax.x - vMin.x, vMax.y - vMin.y);

            Render::CResource* ViewportTex = ResourceManager->GetResource(ViewportView->RenderTarget.Texture);
            CASSERT(ViewportTex && ViewportTex->GetDesc().Flags & Render::EResourceFlags::Texture);
            const auto& TexDesc = ViewportTex->GetDesc().Texture;

            ImVec2 uv0(0, 0);
            ImVec2 uv1(
                (Viewport->BottomRightCorner.X /*+ 0.5f*/) / (float)TexDesc.Width,
                (Viewport->BottomRightCorner.Y /*+ 0.5f*/) / (float)TexDesc.Height
            );
            
            ImGui::Image(m_ViewportRenderTargetsDescriptorSet[i], vSize, uv0, uv1);
            Viewport->UpperLeftCorner = { 0, 0 };
            Viewport->BottomRightCorner = { vSize.x , vSize.y };
        }
        ImGui::End();
        m_ViewportOpened[i] = IsOpened;
    }

    ImGui::PopStyleVar(3);
}

void CEditorApplication::ShowWorldOutliner()
{
    if (ImGui::Begin("World Outliner"))
    {
    }
    ImGui::End();
}

void CEditorApplication::ShowProperties()
{
    if (ImGui::Begin("Properties"))
    {
    }
    ImGui::End();
}

void CEditorApplication::ShowContentBrowser()
{
    if (ImGui::Begin("ContentBrowser"))
    {
    }
    ImGui::End();
}

} // namespace Cyclone
