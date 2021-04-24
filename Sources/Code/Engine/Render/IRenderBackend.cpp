#include "IRenderBackend.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

namespace Render
{

static IRenderBackend* GCurrentRenderBackend = nullptr;

} // namespace Render

ENGINE_API Render::IRenderBackend* GEngineGetCurrentRenderBackend()
{
    return Render::GCurrentRenderBackend;
}

ENGINE_API void GEngineSetCurrentRenderBackend(Render::IRenderBackend* RenderBackend)
{
    CASSERT(Render::GCurrentRenderBackend == nullptr);
    Render::GCurrentRenderBackend = RenderBackend;
}

} // namespace Cyclone::Render
