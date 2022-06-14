#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{
class CScene;
}

namespace Cyclone::Render
{

class CRenderSceneView;

class ENGINE_API CRenderScene
{
public:
    DISABLE_COPY_ENABLE_MOVE(CRenderScene);

    CRenderScene() = default;
    virtual ~CRenderScene() = default;

    // list of static/dynamic primitives
    // post processing settings
    // culled objects
    // decals
    // lights
    // etc

    // #todo_vk check for duplicates
    C_STATUS AddSceneView(Ptr<CRenderSceneView> SceneView) { m_Views.emplace_back(std::move(SceneView)); return C_STATUS::C_STATUS_OK; }

public:
    CScene* m_Scene;

    Vector<Ptr<CRenderSceneView>> m_Views;
};

} // namespace Cyclone::Render
