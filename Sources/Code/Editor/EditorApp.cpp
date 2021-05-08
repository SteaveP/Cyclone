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
    // viewport
    if (ImGui::Begin("Viewport"))
    {
        // #todo_ui
        ImGui::End();
    }

    // content browser
    // world outliner
    // properties grid

    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
