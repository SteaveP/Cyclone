#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"
#include "Engine/Render/Handle.h"

namespace Cyclone
{

class IWindow;
class IUISubsystem;

namespace Render
{ 
    class IRendererBackend;
    class CCommandBuffer;
    class CResource;
    class CResourceView;
    enum class EImageLayoutType;
}

class ENGINE_API ImGUIRenderer
{
public:
    ~ImGUIRenderer() = default;

    virtual C_STATUS OnInit(void* Instance, IUISubsystem* UISubsystem,  Render::IRendererBackend* Backend, IWindow* Window) = 0;
    virtual C_STATUS OnShutdown(void* Instance, IWindow* Window) = 0;

    virtual C_STATUS OnFrame(void* Instance) = 0;
    virtual C_STATUS OnRender(void* Instance, Render::CCommandBuffer* CommandBuffer) = 0;

    virtual RawPtr RegisterTexture(Render::CHandle<Render::CResourceView> View, Render::EImageLayoutType ExpectedLayout) = 0;
    virtual void UnRegisterTexture(Render::CHandle<Render::CResourceView> View, RawPtr Descriptor) = 0;
};

} // namespace Cyclone
