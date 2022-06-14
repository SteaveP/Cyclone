#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

class IRenderer;
class IWindow;
class IModule;

namespace Render
{

class WindowContext;
class CCommandQueue;
class CCommandBuffer;
class CTexture;

class ENGINE_API IRendererBackend
{
public:
    virtual ~IRendererBackend() = default;

    virtual C_STATUS Init(IRenderer* Renderer) = 0;
    virtual C_STATUS Shutdown() = 0;

    virtual C_STATUS BeginRender() = 0;
    virtual C_STATUS EndRender() = 0;

    virtual WindowContext* CreateWindowContext(IWindow* Window) = 0;
    virtual CCommandQueue* CreateCommandQueue() = 0;
    virtual CCommandBuffer* CreateCommandBuffer() = 0;

    virtual CTexture* CreateTexture() = 0;
    virtual IRenderer* GetRenderer() const = 0;
};

} // namespace Render

ENGINE_API Render::IRendererBackend* GEngineGetCurrentRenderBackend();
ENGINE_API void GEngineSetCurrentRenderBackend(Render::IRendererBackend* RenderBackend);

} // namespace Cyclone
