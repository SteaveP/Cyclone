#include "RenderBackendVulkanModule.h"
#include "Engine/Render/Backend/Vulkan/RenderBackendVulkan.h"

#include "Engine/Render/Backend/Vulkan/CommonVulkan.h"

#include "UI/ImGUIRendererVulkan.h"

namespace Cyclone
{

IModule* CreateRenderBackendModule()
{
    return new RenderBackendVulkanModule();
}

namespace Render
{

static RenderBackendVulkan GRenderBackendVulkan{};

} // namespace Render

ImGUIRenderer* CreateImGUIRenderer()
{
    return new Render::ImGUIRendererVulkan();
}

C_STATUS RenderBackendVulkanModule::OnRegister()
{
#ifdef _DEBUG
    printf("Module: RenderBackendVulkanModule registered\n");
#endif

    CASSERT(GEngineGetCurrentRenderBackend() == nullptr);
    GEngineSetCurrentRenderBackend(&Render::GRenderBackendVulkan);
    return C_STATUS::C_STATUS_OK;
}

C_STATUS RenderBackendVulkanModule::OnUnRegister()
{
#ifdef _DEBUG
    printf("Module: RenderBackendVulkanModule registered\n");
#endif

    CASSERT(GEngineGetCurrentRenderBackend() == &Render::GRenderBackendVulkan);
    GEngineSetCurrentRenderBackend(nullptr);
    return C_STATUS::C_STATUS_OK;
}

} // namespace Cyclone
