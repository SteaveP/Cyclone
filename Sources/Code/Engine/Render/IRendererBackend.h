#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

class IRenderer;
class IWindow;
class IModule;

namespace Render
{

class CWindowContext;
class CCommandQueue;
class CCommandBuffer;
class CTexture;
class CBuffer;
class CShader;
class CPipeline;
struct CDeviceHandle;
enum class EFormatType;
enum class EImageAspectType;

class ENGINE_API IRendererBackend
{
public:
    virtual ~IRendererBackend() = default;

    virtual C_STATUS Init(IRenderer* Renderer) = 0;
    virtual C_STATUS Shutdown() = 0;

    virtual C_STATUS BeginRender() = 0;
    virtual C_STATUS EndRender() = 0;

    virtual CWindowContext* CreateWindowContext(IWindow* Window) = 0;
    virtual CCommandQueue* CreateCommandQueue() = 0;
    virtual CCommandBuffer* CreateCommandBuffer() = 0;
    virtual CShader* CreateShader() = 0;
    virtual CPipeline* CreatePipeline() = 0;

    virtual CTexture* CreateTexture() = 0;
    virtual RawPtr CreateTextureView(CDeviceHandle Device, CTexture* Texture, uint32 StartMip, uint32 MipLevels, EFormatType Format, EImageAspectType AspectMask) = 0;
    virtual void DestroyTextureView(CDeviceHandle Device, CTexture* Texture, RawPtr TextureView) = 0;
    
    virtual CBuffer* CreateBuffer() = 0;
    virtual RawPtr CreateBufferView(CDeviceHandle Device, CBuffer* Buffer, uint64 Offset, uint64 Size, EFormatType Format) = 0;
    virtual void DestroyBufferView(CDeviceHandle Device, CBuffer* Buffer, RawPtr BufferView) = 0;

    virtual IRenderer* GetRenderer() const = 0;

    virtual void WaitGPU() = 0;
};

} // namespace Render

ENGINE_API Render::IRendererBackend* GEngineGetCurrentRenderBackend();
ENGINE_API void GEngineSetCurrentRenderBackend(Render::IRendererBackend* RenderBackend);

} // namespace Cyclone
