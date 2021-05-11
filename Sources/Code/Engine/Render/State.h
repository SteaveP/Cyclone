#pragma once

#include "Engine/EngineModule.h"

namespace Cyclone::Render
{

enum class DepthStencilState
{

};

enum class RasterizerState
{

};

enum class BlendState
{

};

enum class PixelType
{
    RGBA8,
    RGBA8_SRGB,
    RG8,
    R8,
    R10G10B10A2,
    R11G11B10,
    RGBA16,
    RG16,
    R16,
    RGBA32,
    RGB32,
    RG32,
    R32,
    BC1,
    BC2,
    BC3,
    BC4,
    BC5,
    BC6,
    BC7,
    // Depth
    D_16,
    D_24,
    D_32,
    //
    Count
};

} // namespace Cyclone::Render
