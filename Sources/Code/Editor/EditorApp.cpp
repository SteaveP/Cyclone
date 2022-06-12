#include "EditorApp.h"

#include "Engine/UI/CommonUI.h"

namespace Cyclone
{


C_STATUS EditorApplication::OnInit()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS EditorApplication::OnUpdate()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS EditorApplication::OnRender()
{
    return C_STATUS::C_STATUS_OK;
}

C_STATUS EditorApplication::OnUpdateUI()
{
    ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

    ShowMenu();

    ShowViewport();
    ShowWorldOutliner();
    ShowProperties();
    ShowContentBrowser();

    return C_STATUS::C_STATUS_OK;
}
void EditorApplication::ShowMenu()
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
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void EditorApplication::ShowViewport()
{
//     const ImGuiViewport* viewport = ImGui::GetMainViewport();
//     ImGui::SetNextWindowPos(viewport->WorkPos);
//     ImGui::SetNextWindowSize(viewport->WorkSize);
//     ImGui::SetNextWindowViewport(viewport->ID);
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//     ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("Viewport"))
    {
    }
    ImGui::End();
}

void EditorApplication::ShowWorldOutliner()
{
    if (ImGui::Begin("World Outliner"))
    {
    }
    ImGui::End();
}

void EditorApplication::ShowProperties()
{
    if (ImGui::Begin("Properties"))
    {
    }
    ImGui::End();
}

void EditorApplication::ShowContentBrowser()
{
    if (ImGui::Begin("ContentBrowser"))
    {
    }
    ImGui::End();
}

} // namespace Cyclone
