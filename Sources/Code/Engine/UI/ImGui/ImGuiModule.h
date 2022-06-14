#pragma once

#include "Engine/Framework/IModule.h"

namespace Cyclone
{

class ImGUIPlatform;
class ImGUIRenderer;

class ENGINE_API ImGUIModule : public IModule
{
public:
    ImGUIModule() : IModule("ImGUIModule") {}

    virtual C_STATUS OnRegister() override;
    virtual C_STATUS OnUnRegister() override;

    void SetRenderer(ImGUIRenderer* Renderer) { m_Renderer = Renderer; }
    void SetPlatform(ImGUIPlatform* Platform) { m_Platform = Platform; }

protected:
    ImGUIRenderer* m_Renderer = nullptr;
    ImGUIPlatform* m_Platform = nullptr;
};

} // namespace Cyclone
