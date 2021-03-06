#pragma once

#include "Engine/Framework/IModule.h"
#include "RenderBackendVulkanModuleDefines.h"

namespace Cyclone
{

class ImGUIRenderer;


class CRenderBackendVulkanModule : public IModule
{
public:
    CRenderBackendVulkanModule() : IModule("RenderBackendVulkanModule") {}

    virtual C_STATUS OnRegister() override;
    virtual C_STATUS OnUnRegister() override;
};

RENDER_BACKEND_VK_API IModule* CreateRenderBackendModule();
RENDER_BACKEND_VK_API ImGUIRenderer* CreateImGUIRenderer();

} // namespace Cyclone
