#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

#include "Engine/Render/Handle.h"

namespace DelegateLib
{
    template<typename Param1>
    class MulticastDelegate1;
}

namespace Cyclone
{

class IRenderer;
class IWindow;
class IModule;
struct Vec4;

namespace Render
{

class CWindowContext;
class CCommandQueue;
class CCommandBuffer;
class CResource;
class CResourceView;
class CBuffer;
class CBufferView;
class CShader;
class CPipelineState;
class CUploadQueue;
struct CDeviceHandle;
class IBindlessManager;
class IResourceManager;
enum class EFormatType;
enum class EImageAspectType;

class ENGINE_API IRendererBackend
{
public:
    DISABLE_COPY_ENABLE_MOVE(IRendererBackend);

    IRendererBackend() = default;
    virtual ~IRendererBackend() = default;

    virtual C_STATUS Init(IRenderer* Renderer) = 0;
    virtual C_STATUS Shutdown() = 0;

    virtual C_STATUS BeginRender() = 0;
    virtual C_STATUS EndRender() = 0;

    virtual IRenderer* GetRenderer() const = 0;
    virtual IBindlessManager* GetBindlessManager(CDeviceHandle DeviceHandle) = 0;
    virtual IResourceManager* GetResourceManager(CDeviceHandle DeviceHandle) = 0;
    virtual CUploadQueue* GetUploadQueue(CDeviceHandle DeviceHandle) = 0;
    virtual void WaitGPU() = 0;

    virtual CHandle<CWindowContext> CreateWindowContext() = 0;
    virtual void DestroyWindowContext(CHandle<CWindowContext> Handle) = 0;
    virtual CWindowContext* GetWindowContext(CHandle<CWindowContext> Handle) = 0;
    
    // Device delegates
    using CDeviceDelegate = DelegateLib::MulticastDelegate1<CDeviceHandle>;
    virtual CDeviceDelegate* GetOnDeviceCreatedDelegate() = 0;
    virtual CDeviceDelegate* GetOnDeviceRemovedDelegate() = 0;
    
    // Profiling 
    virtual void ProfileGPUEventBegin(class CCommandBuffer* CommandBuffer, const char* Name, const Vec4& Color) = 0;
    virtual void ProfileGPUEventEnd(class CCommandBuffer* CommandBuffer) = 0;

    virtual void ProfileGPUEventBegin(class CCommandQueue* CommandQueue, const char* Name, const Vec4& Color) = 0;
    virtual void ProfileGPUEventEnd(class CCommandQueue* CommandQueue) = 0;
};

} // namespace Render

ENGINE_API Render::IRendererBackend* GEngineGetCurrentRenderBackend();
ENGINE_API void GEngineSetCurrentRenderBackend(Render::IRendererBackend* RenderBackend);

} // namespace Cyclone
