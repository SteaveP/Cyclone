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

class CSceneRenderer;

class CTexture;
class CBuffer;
class CPipeline;
class CPipelineDesc;
class CShader;
class CCommandBuffer;
class CCommandQueue;
class CWindowContext;
class IRendererBackend;

class CRenderScene;
class CRenderSceneView;

//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////

#define IMPEMENT_ENUM_BITFIELD(EnumType)                                                \
    inline EnumType operator | (EnumType a, EnumType b)                                 \
    {                                                                                   \
        return static_cast<EnumType>(static_cast<uint32>(a) | static_cast<uint32>(b));  \
    }                                                                                   \
                                                                                        \
    inline EnumType& operator |= (EnumType& a, EnumType b)                              \
    {                                                                                   \
        a = a | b;                                                                      \
        return a;                                                                       \
    }                                                                                   \
                                                                                        \
    inline uint32 operator & (EnumType a, EnumType b)                                   \
    {                                                                                   \
        return static_cast<uint32>(a) & static_cast<uint32>(b);                         \
    }                                                                                   \

enum class CommandQueueType
{
    Graphics,
    AsyncCompute,
    Present,
    Count
};


enum class EImageLayoutType
{
    Undefined,
    Default,
    ColorAttachment,
    // #todo_vk depth stencil
    ShaderReadOnly,
    TransferSrc,
    TransferDst,
    ReadOnly,
    Present,
    Count
};

enum class ERenderTargetLoadOp
{
    Load,
    Clear,
    DontCare,
    Count
};

enum class ERenderTargetStoreOp
{
    Store,
    DontCare,
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

enum class EImageUsageType
{   
    None                = 0,
    TransferScr         = (1 << 0),
    TransferDst         = (1 << 1),
    Sampled             = (1 << 2),
    Storage             = (1 << 3),
    ColorAttachment     = (1 << 4),
    DepthStencil        = (1 << 5),
    ShaderResourceView  = (1 << 6),
    Count               = 7
};
IMPEMENT_ENUM_BITFIELD(EImageUsageType);

enum class EBufferUsageType
{
    None                = 0,
    TransferScr         = (1 << 0),
    TransferDst         = (1 << 1),
    Uniform             = (1 << 2),
    Storage             = (1 << 3),
    Index               = (1 << 4),
    Vertex              = (1 << 5),
    Indirect            = (1 << 6),
    Count               = 7
};
IMPEMENT_ENUM_BITFIELD(EBufferUsageType);

enum class EImageAspectType
{
    None      = 0,
    Color     = (1 << 0),
    Depth     = (1 << 1),
    Stencil   = (1 << 2),
    Metadata  = (1 << 3),
    Count     = 4
};
IMPEMENT_ENUM_BITFIELD(EImageAspectType);

enum class EResourceFlags
{
    None            = 0,
    Mappable        = (1 << 0),

    Count           = 1
};
IMPEMENT_ENUM_BITFIELD(EResourceFlags);

union CClearColor
{
    Vec4 Color;
    Vec2 DepthStencil;

    CClearColor() : CClearColor({ 0,0,0,0 }) {};
    CClearColor(Vec4 ColorArg) : Color(ColorArg) {}
    CClearColor(Vec2 DepthStencilArg) : DepthStencil(DepthStencilArg) {}

    bool operator==(const CClearColor& Other) const noexcept { return Color == Other.Color; }
};

struct CDeviceHandle
{
    static const uint16 InvalidHandle = static_cast<uint16>(-1);

    uint16 PhysicalDeviceHandle = InvalidHandle;
    uint16 LogicalDeviceHandle = InvalidHandle;

    bool IsPhysicalDeviceHandleValid() const { return PhysicalDeviceHandle != InvalidHandle; }
    bool IsLogicalDeviceHandleValid() const { return LogicalDeviceHandle != InvalidHandle; }

    bool operator ==(const CDeviceHandle& Other) const noexcept
    {
        return PhysicalDeviceHandle == Other.PhysicalDeviceHandle
            && LogicalDeviceHandle == Other.LogicalDeviceHandle;
    }

    uint64 GetHash() const noexcept { return uint64(PhysicalDeviceHandle) << 32l | uint64(LogicalDeviceHandle) << 48l; }
};

class CTextureView
{
public:
    void* PlatformDataPtr = nullptr;
    // #todo_vk is it enough one class for all views (CBV, RTV, DSV, UAV) ? What about RTX?
};

class CRenderTarget
{
public:
    Ptr<CTexture> Texture = nullptr;

    // #todo_vk Additional views for resource
    Ptr<CTextureView> RenderTargetView;
    Ptr<CTextureView> DepthStencilView;
    Ptr<CTextureView> ShaderResourceView;
    Ptr<CTextureView> UnorderedAccessView;

    //bool operator ==(const CRenderTarget& Other) const noexcept { return true; }
};

class CRenderTargetSlot
{
public:
    Ptr<CRenderTarget> RenderTarget;
    
    EImageLayoutType InitialLayout = EImageLayoutType::Undefined;
    EImageLayoutType Layout = EImageLayoutType::Undefined;
    EImageLayoutType FinalLayout = EImageLayoutType::Undefined;
    ERenderTargetLoadOp LoadOp = ERenderTargetLoadOp::Load;
    ERenderTargetStoreOp StoreOp = ERenderTargetStoreOp::Store;
    ERenderTargetLoadOp StencilLoadOp = ERenderTargetLoadOp::Load;
    ERenderTargetStoreOp StencilStoreOp = ERenderTargetStoreOp::Store;

    // #todo_vk other state?

    CClearColor ClearValue;

    bool operator ==(const CRenderTargetSlot& Other) const noexcept {
        return ClearValue == Other.ClearValue;
    }
};

class CRenderTargetSet
{
public:
    static const uint32 MaxRenderTargets = 10;

    uint32 RenderTargetsCount = 0;
    Array<CRenderTargetSlot, MaxRenderTargets> RenderTargets;

    CRenderTargetSlot DepthScentil;

    bool operator ==(const CRenderTargetSet& Other) const noexcept
    {
        return RenderTargetsCount == Other.RenderTargetsCount &&
            RenderTargets == Other.RenderTargets &&
            DepthScentil == Other.DepthScentil;
    }

};

class CRenderPass
{
public:
    CRenderTargetSet RenderTargetSet;
    Vec4 ViewportExtent;

    bool operator ==(const CRenderPass& Other) const noexcept { return RenderTargetSet == Other.RenderTargetSet && ViewportExtent == Other.ViewportExtent; }
};

class CRasterizerState
{
public:
    EPolygonFillMode PolygonFillMode = EPolygonFillMode::Fill;
    ECullMode CullMode = ECullMode::Back;
    bool IsfrontFaceCounterClockwise = true;
    bool DepthBiasEnable = false;
    bool DepthClampEnable = false;
    float LineWidth = 1.f;
    float DepthBiasClamp = 0.f;
    float DepthBiasConstantFactor = 0.f;
    float DepthBiasSlopeFactor = 0.f;
};

class CDepthStencilState
{
public:
    bool DepthTestEnable = false;
    bool DepthWriteEnable = false;
    ECompareOp DepthCompareOp = ECompareOp::Greater;
    bool  DepthBoundsTestEnable = false;
    float MinDepthBounds = 0.0f;
    float MaxDepthBounds = 1.0f;
    bool StencilTestEnable = false;
};

class CBlendState
{
public:
    uint16 BlendEnable = false;
    uint16 ColorWriteMask = 0xF;
    uint16 srcColorBlendFactor = 0;
    uint16 dstColorBlendFactor = 0;
    uint16 colorBlendOp = 0;
    uint16 srcAlphaBlendFactor = 0;
    uint16 dstAlphaBlendFactor = 0;
    uint16 alphaBlendOp = 0;
};

} // namespace Cyclone::Render
