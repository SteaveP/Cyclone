#pragma once

#include "Engine/EngineModule.h"

namespace Cyclone::Render
{

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

enum class DepthStencilState
{
    Disabled,
    Count
};

enum class RasterizerState
{
    Default,
    Count
};

enum class BlendState
{
    Disabled,
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

} // namespace Cyclone::Render
