#include "Mesh.h"

namespace Cyclone::Render
{

CVertexLayoutDescription VertexPosColorUV::GetLayoutDescription()
{
    CVertexLayoutDescription Desc{};

    Desc.Bindings.emplace_back(CVertexBindingDescription{
        .Index = 0,
        .Stride = sizeof(VertexPosColorUV),
        .BindingRate = EVertexBindingRate::Vertex
    });

    // Pos
    Desc.Attributes.emplace_back(CVertexAttributeDescription{
        .BindingIndex = 0,
        .SlotIndex = 0,
        .Format = EFormatType::RGB32_Float,
        .Offset = offsetof(VertexPosColorUV, Pos),
        });

    // Color
    Desc.Attributes.emplace_back(CVertexAttributeDescription{
        .BindingIndex = 0,
        .SlotIndex = 1,
        .Format = EFormatType::RGB32_Float,
        .Offset = offsetof(VertexPosColorUV, Color),
        });

    // TexCoord
    Desc.Attributes.emplace_back(CVertexAttributeDescription{
        .BindingIndex = 0,
        .SlotIndex = 2,
        .Format = EFormatType::RG32_Float,
        .Offset = offsetof(VertexPosColorUV, TexCoord),
        });

    return Desc;
}

} // namespace Cyclone::Render
