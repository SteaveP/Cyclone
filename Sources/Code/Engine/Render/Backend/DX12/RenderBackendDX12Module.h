#pragma once

#include "Engine/Framework/IModule.h"
#include "RenderBackendDX12ModuleDefines.h"

namespace Cyclone
{

class ImGUIRenderer;

class RENDER_BACKEND_DX12_API RenderBackendDX12Module : public IModule
{
public:
    virtual C_STATUS OnRegister() override;
    virtual C_STATUS OnUnRegister() override;
};

RENDER_BACKEND_DX12_API ImGUIRenderer* CreateImGUIRenderer();

} // namespace Cyclone
