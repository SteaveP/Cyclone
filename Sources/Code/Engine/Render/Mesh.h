#pragma once

#include "Engine/Render/CommonRender.h"

namespace Cyclone::Render
{

struct CVertexBindingDescription
{
    uint32 Index = 0;
    uint32 Stride = 0;
    EVertexBindingRate BindingRate = EVertexBindingRate::Vertex;

    bool operator ==(const CVertexBindingDescription& Other) const noexcept = default;

};

struct CVertexAttributeDescription
{
    uint32 BindingIndex = 0;
    uint32 SlotIndex = 0;
    EFormatType Format = EFormatType::Undefined;
    uint32 Offset = 0;

    bool operator ==(const CVertexAttributeDescription& Other) const noexcept = default;
};

struct CVertexLayoutDescription
{
    Vector<CVertexBindingDescription> Bindings;
    Vector<CVertexAttributeDescription> Attributes;

    bool operator ==(const CVertexLayoutDescription& Other) const noexcept = default;
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
