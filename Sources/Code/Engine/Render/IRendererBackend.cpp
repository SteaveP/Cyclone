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
    CASSERT(Render::GCurrentRenderBackend == nullptr);
    Render::GCurrentRenderBackend = RenderBackend;
}

} // namespace Cyclone::Render
