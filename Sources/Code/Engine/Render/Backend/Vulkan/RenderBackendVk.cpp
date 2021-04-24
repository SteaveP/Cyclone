#include "RenderBackendVk.h"

#include "Engine/Framework/IModule.h"

#include "Common/CommonVulkan.h"

namespace Cyclone
{

namespace Render
{

static RenderBackendVulkan GRenderBackendVulkan{};

} // namespace Render

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

} //namespace Cyclone
