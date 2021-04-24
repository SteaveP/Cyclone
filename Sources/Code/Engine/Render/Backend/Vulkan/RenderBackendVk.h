#pragma once

#include "Engine/Framework/IModule.h"
#include "Engine/Render/IRenderBackend.h"
#include "RenderBackendVkModule.h"

namespace Cyclone
{

namespace Render
{

class RENDER_BACKEND_VK_API RenderBackendVulkan : public IRenderBackend
{
    public:
};

} // namespace Render

class RENDER_BACKEND_VK_API RenderBackendVulkanModule : public IModule
{
public:
    virtual C_STATUS OnRegister() override;
    virtual C_STATUS OnUnRegister() override;
};

} // namespace Cyclone
