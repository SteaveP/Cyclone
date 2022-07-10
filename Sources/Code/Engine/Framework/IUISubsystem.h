#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"
#include "Engine/Render/Handle.h"

namespace Cyclone
{

namespace Render { class CCommandBuffer; class CResource; class CResourceView; enum class EImageLayoutType; }

class IApplication;

class ENGINE_API IUISubsystem
{
public:
    DISABLE_COPY_ENABLE_MOVE(IUISubsystem);

    IUISubsystem() = default;
    virtual ~IUISubsystem() = default;

    virtual C_STATUS Init(IApplication* App, float Dpi = 96.f) = 0;
    virtual void Shutdown() noexcept = 0;

    virtual C_STATUS OnFrame() = 0;
    virtual C_STATUS OnRender(Render::CCommandBuffer* CommandBuffer) = 0;

    virtual C_STATUS OnEndFrame() = 0;

    virtual C_STATUS OnWindowMessage(void* Params) = 0;
    virtual C_STATUS OnDPIChanged(float NewDPI, float OldDPI) = 0;

    virtual RawPtr RegisterTexture(Render::CHandle<Render::CResourceView> View, Render::EImageLayoutType ExpectedLayout) = 0;
    virtual void UnRegisterTexture(Render::CHandle<Render::CResourceView> View, RawPtr Descriptor) = 0;
};

ENGINE_API IUISubsystem* GEngineGetCurrentUISubsystem();
ENGINE_API void GEngineSetCurrentUISubsystem(IUISubsystem* UIModule);

} // namespace Cyclone
