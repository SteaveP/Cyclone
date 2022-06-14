#pragma once

#include "Engine/EngineModule.h"

namespace Cyclone
{
class CSceneViewport;
} // namespace Cyclone


namespace Cyclone::Render
{

class CWindowContext;
class CRenderScene;

class ENGINE_API CRenderSceneView
{
public:
    virtual ~CRenderSceneView() {}

public:
    CSceneViewport* Viewport;

    CWindowContext* RenderWindowContext;
    CRenderScene* RenderScene;
};

} // namespace Cyclone::Render
