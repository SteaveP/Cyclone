#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Math.h"
#include "Engine/EngineModule.h"

#include "State.h"

namespace Cyclone::Render
{

const int MAX_FRAMES_IN_FLIGHT = 2;

//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////

class CTexture;
class CBuffer;
class CCommandBuffer;
class CCommandQueue;
class CWindowContext;
class IRendererBackendResource;

class CRenderScene;
class CRenderSceneView;

//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////

enum class CommandQueueType
{
    Graphics,
    AsyncCompute,
    Present,
    Count
};

enum class EImageLayout
{
    Undefined,
    Default,
    Count
};

enum class EImageType
{
    Type1D,
    Type2D,
    Type3D,
    Count
};
enum class ETilingType
{
    Optimal,
    Linear,
    Count
};

using RawPtr = void*;

union CClearColor
{
    Vec4 Color;
    Vec2 DepthStencil;

    CClearColor(Vec4 ColorArg) : Color(ColorArg) {}
    CClearColor(Vec2 DepthStencilArg) : DepthStencil(DepthStencilArg) {}
};

class ResourceView
{
public:
    // #todo_vk is it enough one class for all views (CBV, RTV, DSV, UAV) ? What about RTX?
};

class CRenderTarget
{
public:
    Ptr<CTexture> Texture = nullptr;

    // #todo_vk Additional views for resource
    Ptr<ResourceView> RenderTargetView;
    Ptr<ResourceView> DepthStencilView;
    Ptr<ResourceView> ShaderResourceView;
    Ptr<ResourceView> UnorderedAccessView;
};

class CRenderTargetBind
{
public:
    Ptr<CRenderTarget> RenderTarget;
    
    // #todo_vk other state

    Optional<CClearColor> ClearValue;
};

class CRenderTargetSet
{
public:
    static const uint32 MaxRenderTargets = 10;

    uint32 RenderTargetsCount = 0;
    Array<CRenderTargetBind, MaxRenderTargets> RenderTargets;

    CRenderTargetBind DepthScentil;
};

class CRenderPass
{
public:
    CRenderTargetSet RenderTargetSet;
    Vec4 ViewportExtent;

};

} // namespace Cyclone::Render
