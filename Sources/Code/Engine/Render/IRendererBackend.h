#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

class IRenderer;
class IWindow;

namespace Render
{

class ENGINE_API IRendererBackend
{
public:
    virtual ~IRendererBackend() = default;

    virtual C_STATUS Init(IRenderer* Renderer) = 0;
    virtual C_STATUS Shutdown() = 0;

    virtual C_STATUS BeginRender() = 0;
    virtual C_STATUS Render() = 0;
    virtual C_STATUS EndRender() = 0;

    virtual uint32_t GetCurrentFrame() const = 0;
    virtual uint32_t GetCurrentLocalFrame() const = 0;
};

} // namespace Render

ENGINE_API Render::IRendererBackend* GEngineGetCurrentRenderBackend();
ENGINE_API void GEngineSetCurrentRenderBackend(Render::IRendererBackend* RenderBackend);

} // namespace Cyclone
