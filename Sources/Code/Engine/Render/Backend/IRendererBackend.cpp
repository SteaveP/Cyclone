#include "IRendererBackend.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

namespace Render
{

static IRendererBackend* GCurrentRenderBackend = nullptr;

} // namespace Render

ENGINE_API Render::IRendererBackend* GEngineGetCurrentRenderBackend()
{
    return Render::GCurrentRenderBackend;
}

ENGINE_API void GEngineSetCurrentRenderBackend(Render::IRendererBackend* RenderBackend)
{
    // #todo callback OnRenderBackendChanged?
    CASSERT(RenderBackend == nullptr || Render::GCurrentRenderBackend == nullptr);
    Render::GCurrentRenderBackend = RenderBackend;
}

} // namespace Cyclone
