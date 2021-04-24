#pragma once

#include "Engine/EngineModule.h"

namespace Cyclone
{

class IRenderer;

namespace Render
{

class ENGINE_API IRendererBackend
{
public:
    virtual ~IRendererBackend() = default;

    virtual C_STATUS Init(IRenderer* Renderer) = 0;
    virtual C_STATUS Shutdown() = 0;

    virtual C_STATUS Render() = 0;
};

} // namespace Render

ENGINE_API Render::IRendererBackend* GEngineGetCurrentRenderBackend();
ENGINE_API void GEngineSetCurrentRenderBackend(Render::IRendererBackend* RenderBackend);

} // namespace Cyclone
