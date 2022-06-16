#pragma once

#include "Engine/Render/Common.h"

namespace Cyclone::Render
{

struct CVertexBindingDescription
{
    uint32 Index = 0;
    uint32 Stride = 0;
    EVertexBindingRate BindingRate = EVertexBindingRate::Vertex;
};

struct CVertexAttributeDescription
{
    uint32 BindingIndex = 0;
    uint32 SlotIndex = 0;
    EFormatType Format = EFormatType::Undefined;
    uint32 Offset = 0;
};

struct CVertexLayoutDescription
{
    Vector<CVertexBindingDescription> Bindings;
    Vector<CVertexAttributeDescription> Attributes;
};

struct VertexPosColorUV
{
    Vec3 Pos;
    Vec3 Color;
    Vec2 TexCoord;

    static CVertexLayoutDescription GetLayoutDescription();

    bool operator == (const VertexPosColorUV& other) const noexcept { return Pos == other.Pos && Color == other.Color && TexCoord == other.TexCoord; }
};


} // namespace Cyclone::Render
