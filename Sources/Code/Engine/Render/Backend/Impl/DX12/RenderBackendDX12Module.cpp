#include "RenderBackendDX12Module.h"
#include "RenderBackendDX12.h"

#include "Engine/Utils/Log.h"

#include "CommonDX12.h"
#include "UI/ImGUIRendererDX12.h"

namespace Cyclone
{

IModule* CreateRenderBackendModule()
{
    return new RenderBackendDX12Module();
}

namespace Render
{

static RenderBackendDX12 GRenderBackendDX12{};

} // namespace Render

ImGUIRenderer* CreateImGUIRenderer()
{
    return new Render::ImGUIRendererDX12();
}

C_STATUS RenderBackendDX12Module::OnRegister()
{
    LOG_INFO("Module: RenderBackendDX12Module registered");

    CASSERT(GEngineGetCurrentRenderBackend() == nullptr);
    GEngineSetCurrentRenderBackend(&Render::GRenderBackendDX12);
    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendDX12Module::OnUnRegister()
{
    LOG_INFO("Module: RenderBackendDX12Module registered");

    CASSERT(GEngineGetCurrentRenderBackend() == &Render::GRenderBackendDX12);
    GEngineSetCurrentRenderBackend(nullptr);
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
