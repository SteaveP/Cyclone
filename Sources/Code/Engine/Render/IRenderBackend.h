#pragma once

#include "Engine/EngineModule.h"

namespace Cyclone
{

namespace Render
{

class ENGINE_API IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;
};

} // namespace Render

ENGINE_API Render::IRenderBackend* GEngineGetCurrentRenderBackend();
ENGINE_API void GEngineSetCurrentRenderBackend(Render::IRenderBackend* RenderBackend);

} // namespace Cyclone
