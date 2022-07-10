#include "RenderScene.h"
#include "RenderSceneView.h"

namespace Cyclone::Render
{

CRenderScene::CRenderScene() = default;
CRenderScene::~CRenderScene() = default;

C_STATUS CRenderScene::RemoveSceneView(CRenderSceneView* SceneView)
{
    for (auto it = m_Views.begin(); it != m_Views.end(); ++it)
    {
        if (it->get() == SceneView)
        {
            m_Views.erase(it);
            return C_STATUS::C_STATUS_OK;
        }
    }

    return C_STATUS::C_STATUS_INVALID_ARG;
}

} // namespace Cyclone::Render
