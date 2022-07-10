#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Math.h"
#include "Engine/EngineModule.h"

#include "Handle.h"

namespace Cyclone { class IRenderer; }

namespace Cyclone::Render
{

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////
class IRendererBackend;

class CCommandQueue;
class CCommandBuffer;
class CWindowContext;
class CUploadQueue;

class CResource;
class CResourceView;

class CShader;
class CPipelineState;
class CPipelineStateDesc;

//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////

#define GET_CONFIG_RENDER() GET_CONFIG()["Render"]

enum class PipelineType
{
    Graphics,
    Compute,
    Raytracing,
    Count
};
enum class PrimitiveTopologyType
{
    TriangleList,
    Count
};

enum class EVertexBindingRate
{
    Vertex,
    Instance,
    Count
};

enum class EPolygonFillMode
{
    Fill,
    Line,
    Point,
    Count
};

enum class ECullMode
{
    Disable,
    Back,
    Front,
    Count
};

enum class ECompareOp
{
    Never,
    Less,
    Greater,
    Equal,
    NotEqual,
    LessOrEqual,
    GreaterOrEqual,
    Always,
    Count
};

enum class EFormatType
{
    Undefined,
    RGBA8_UNORM,
    RGBA8_SRGB,
    BGRA8_UNORM,
    BGRA8_SRGB,
    RG8_UNORM,
    R8_UNORM,
    R10G10B10A2_UNORM,
    R11G11B10_Float,
    RGBA16_Float,
    RG16_Float,
    R16_Float,
    R16_UINT,
    RGBA32_Float,
    RGB32_Float,
    RG32_Float,
    R32_Float,
    R32_UINT,
    // BC
    BC1_SRGB,
    BC2_SRGB,
    BC3,
    BC4,
    BC5,
    BC6,
    BC7,
    // Depth
    D_16,
    D_16_S8,
    D_24,
    D_24_S8,
    D_32,
    D_32_S8,
    //
    Count
};

enum class CommandQueueType
{
    Graphics,
    AsyncCompute,
    Present,
    Count
};

enum class EExecutionStageMask
{
    None = 0,
    VertexShader                = bit(0),
    PixelShader                 = bit(1),
    ComputeShader               = bit(2),
    ColorAttachmentOutput       = bit(3),
    DepthStencil                = bit(4),
    Transfer                    = bit(5),
    AllGraphicsCommands         = bit(6),
    AllCommands                 = bit(7),

    Count = 8
};
IMPEMENT_ENUM_BITFIELD(EExecutionStageMask);

enum class EMemoryAccessMask
{
    None = 0,
    ShaderRead                  = bit(0),
    ShaderWrite                 = bit(1),
    ColorAttachmentRead         = bit(2),
    ColorAttachmentWrite        = bit(3),
    DepthStencilRead            = bit(4),
    DepthStencilWrite           = bit(5),
    TransferRead                = bit(6),
    TransferWrite               = bit(7),

    Count = 8
};
IMPEMENT_ENUM_BITFIELD(EMemoryAccessMask);

enum class EImageLayoutType
{
    Undefined,
    Default,
    ColorAttachment,
    DepthStencil,
    DepthStencilReadOnly, // for read in shaders or in depth attachment
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

enum class ETextureType
{
    Type1D,
    Type2D,
    Type3D,
    Count
};

enum class ETextureViewType
{
    Type1D,
    Type2D,
    Type3D,
    // #todo_vk arrays and cube
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
    None = 0,
    TransferSrc              = bit(0),
    TransferDst              = bit(1),
    Sampled                  = bit(2),
    Storage                  = bit(3),
    ColorAttachment          = bit(4),
    DepthStencil             = bit(5),
    ShaderResourceView       = bit(6),

    // #todo_vk_material this values should go to EResoruceUsageType
    Present                  = bit(7),
    DepthStencilRead         = bit(8),

    Count = 9
};
IMPEMENT_ENUM_BITFIELD(EImageUsageType);

enum class EBufferUsageType
{
    None = 0,
    TransferSrc              = bit(0),
    TransferDst              = bit(1),
    Uniform                  = bit(2),
    Storage                  = bit(3),
    Index                    = bit(4),
    Vertex                   = bit(5),
    Indirect                 = bit(6),
    UniformTexel             = bit(7),
    StorageTexel             = bit(8),
    Count = 9
};
IMPEMENT_ENUM_BITFIELD(EBufferUsageType);

enum class EImageAspectType
{
    None = 0,
    Color                    = bit(0),
    Depth                    = bit(1),
    Stencil                  = bit(2),
    Metadata                 = bit(3),
    Count = 4
};
IMPEMENT_ENUM_BITFIELD(EImageAspectType);

enum class EResourceFlags
{
    None = 0,
    Texture                  = bit(0),
    Buffer                   = bit(1),
    Mappable                 = bit(2),
    HeapTypeUpload           = bit(3),
    DenyShaderResource       = bit(4),

    Count = 5
};
IMPEMENT_ENUM_BITFIELD(EResourceFlags);

enum class EDescriptorType
{
    Sampler,
    Buffer,
    BufferUAV,
    Texture,
    TextureUAV,
    Count
};

enum class EDescriptorSetLayoutFlags
{
    None = 0,
    Bindless = (1 << 0),
    Count = 1
};
IMPEMENT_ENUM_BITFIELD(EDescriptorSetLayoutFlags);

enum class EPipelineBarrierAction
{
    Execution,
    GlobalMemory,
    TextureMemory,
    BufferMemory,
    SplitBarrierBegin,
    SplitBarrierEnd,
    Count
};

class CRasterizerState
{
public:
    EPolygonFillMode PolygonFillMode = EPolygonFillMode::Fill;
    ECullMode CullMode = ECullMode::Back;
    bool IsfrontFaceCounterClockwise = false;
    bool DepthBiasEnable = false;
    bool DepthClampEnable = false;
    float LineWidth = 1.f;
    float DepthBiasClamp = 0.f;
    float DepthBiasConstantFactor = 0.f;
    float DepthBiasSlopeFactor = 0.f;

    void DeInit() {}
};

class CDepthStencilState
{
public:
    bool DepthTestEnable = false;
    bool DepthWriteEnable = false;
    ECompareOp DepthCompareOp = ECompareOp::GreaterOrEqual;
    bool  DepthBoundsTestEnable = false;
    float MinDepthBounds = 0.0f;
    float MaxDepthBounds = 1.0f;
    bool StencilTestEnable = false;

    void DeInit() {}
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

    void DeInit() {}
};

// #todo_vk need to add generation because device can be removed but we can't shrink the array
struct CDeviceHandle
{
    static constexpr uint8 InvalidHandle = static_cast<uint8>(-1);

    uint8 DeviceHandle     : 5 = InvalidHandle;
    uint8 PhysDeviceHandle : 3 = InvalidHandle;

    CDeviceHandle() {}
    CDeviceHandle(uint32 DeviceHandle, uint32 PhysDeviceHandle) 
        : DeviceHandle(uint8(DeviceHandle)), PhysDeviceHandle(uint8(PhysDeviceHandle)) {}

    bool IsPhysDeviceHandleValid() const { return PhysDeviceHandle != (InvalidHandle & 0b111); }
    bool IsDeviceHandleValid() const { return DeviceHandle != (InvalidHandle & 0b11111); }
    bool IsValid() const { return IsPhysDeviceHandleValid() && IsDeviceHandleValid(); }

    uint64 GetHash() const noexcept { return uint64(PhysDeviceHandle) << 32l ^ uint64(DeviceHandle) << 48l; }

    bool operator ==(const CDeviceHandle& Other) const noexcept = default;

    template<typename T>
    static CDeviceHandle From(CHandle<T> Handle)
    {
        CASSERT(Handle.IsValid());
        return CDeviceHandle{
            Handle.m_DeviceHandle & 0b11111,
            (Handle.m_DeviceHandle >> 5) & 0b111
        };
    }

    template<typename T>
    CDeviceHandle& Fill(CHandle<T>& Handle)
    {
        Handle.m_DeviceHandle = (DeviceHandle & 0b11111) | ((PhysDeviceHandle & 0b111) << 5);
        return *this;
    }
};
static_assert(sizeof(CDeviceHandle) == 1);

// #todo_vk is this needed?
// General resource class to track usage and deferred deletion
class ENGINE_API IDeviceResource
{
public:
    DISABLE_COPY_ENABLE_MOVE(IDeviceResource);

    IDeviceResource() = default;
    virtual ~IDeviceResource() = default;

    C_STATUS Init(IRendererBackend* Backend, uint64 LastFrameUsed) { m_Backend = Backend; m_LastFrameUsed = LastFrameUsed; return C_STATUS::C_STATUS_OK; }

    virtual void DeInit() = 0;

    virtual void UpdateLastFrameUsed(uint64 LastFrameUsed) { m_LastFrameUsed = LastFrameUsed; }

    uint64 GetLastFrameUsed() const { return m_LastFrameUsed; }
    bool IsPendingDelete() const { return m_IsPendingDelete; }

protected:
    uint64 m_LastFrameUsed = ~0ULL;
    bool m_IsPendingDelete = false;
    IRendererBackend* m_Backend = nullptr;
};


} // namespace Cyclone::Render
