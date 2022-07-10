#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

#include "Engine/Render/CommonRender.h"

namespace Cyclone { class CSceneViewport; }

namespace Cyclone::Render
{

class CWindowContext;
class CRenderScene;
class CRenderTarget;

class ENGINE_API CRenderSceneView
{
public:
    DISABLE_COPY_ENABLE_MOVE(CRenderSceneView);

    CRenderSceneView();
    virtual ~CRenderSceneView();

public:
    CSceneViewport* Viewport = nullptr;

    CWindowContext* RenderWindowContext = nullptr;
    CRenderScene* RenderScene = nullptr;

    CRenderTarget RenderTarget;
};

} // namespace Cyclone::Render
