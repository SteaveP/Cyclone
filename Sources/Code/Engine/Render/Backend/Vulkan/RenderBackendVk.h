#pragma once

#include "Engine/Render/IRendererBackend.h"
#include "RenderBackendVkModule.h"

namespace Cyclone::Render
{

class RENDER_BACKEND_VK_API RenderBackendVulkan : public IRendererBackend
{
public:

    virtual C_STATUS Render() override;
};

} // namespace Cyclone::Render
