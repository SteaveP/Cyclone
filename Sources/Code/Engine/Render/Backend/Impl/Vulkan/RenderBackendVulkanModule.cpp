#include "RenderBackendVulkanModule.h"

#include "Engine/Utils/Log.h"

#include "CommonVulkan.h"
#include "RenderBackendVulkan.h"

#include "UI/ImGUIRendererVulkan.h"

namespace Cyclone
{

IModule* CreateRenderBackendModule()
{
    return new CRenderBackendVulkanModule();
}

namespace Render
{

static CRenderBackendVulkan GRenderBackendVulkan{};

} // namespace Render

ImGUIRenderer* CreateImGUIRenderer()
{
    return new Render::CImGUIRendererVulkan();
}

C_STATUS CRenderBackendVulkanModule::OnRegister()
{
    LOG_INFO("Module: RenderBackendVulkanModule registered");

    CASSERT(GEngineGetCurrentRenderBackend() == nullptr);
    GEngineSetCurrentRenderBackend(&Render::GRenderBackendVulkan);
    return C_STATUS::C_STATUS_OK;
}

C_STATUS CRenderBackendVulkanModule::OnUnRegister()
{
    LOG_INFO("Module: RenderBackendVulkanModule registered");

    CASSERT(GEngineGetCurrentRenderBackend() == &Render::GRenderBackendVulkan);
    GEngineSetCurrentRenderBackend(nullptr);
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
