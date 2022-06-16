#pragma once

#include "Engine/Render/State.h"

namespace Cyclone::Render
{

inline EFormatType ConvertFormatType(VkFormat Format)
{
    switch (Format)
    {
    case VkFormat::VK_FORMAT_UNDEFINED:
        return EFormatType::Undefined;
    case VkFormat::VK_FORMAT_R8G8B8A8_UNORM:
        return EFormatType::RGBA8_UNORM;
    case VkFormat::VK_FORMAT_B8G8R8A8_SRGB:
        return EFormatType::RGBA8_SRGB;
    case VkFormat::VK_FORMAT_R8G8_UNORM:
        return EFormatType::RG8_UNORM;
    case VkFormat::VK_FORMAT_R8_UNORM:
        return EFormatType::R8_UNORM;
    case VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        return EFormatType::R10G10B10A2_UNORM;
    case VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return EFormatType::R11G11B10_Float;
    case VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT:
        return EFormatType::RGBA16_Float;
    case VkFormat::VK_FORMAT_R16G16_SFLOAT:
        return EFormatType::RG16_Float;
    case VkFormat::VK_FORMAT_R16_SFLOAT:
        return EFormatType::R16_Float;
    case VkFormat::VK_FORMAT_R16_UINT:
        return EFormatType::R16_UINT;
    case VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT:
        return EFormatType::RGBA32_Float;
    case VkFormat::VK_FORMAT_R32G32B32_SFLOAT:
        return EFormatType::RGB32_Float;
    case VkFormat::VK_FORMAT_R32G32_SFLOAT:
        return EFormatType::RG32_Float;
    case VkFormat::VK_FORMAT_R32_SFLOAT:
        return EFormatType::R32_Float;
    case VkFormat::VK_FORMAT_R32_UINT:
        return EFormatType::R32_UINT;
    case VkFormat::VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        return EFormatType::BC1_SRGB;
    case VkFormat::VK_FORMAT_BC2_SRGB_BLOCK:
        return EFormatType::BC2_SRGB;
        //case VkFormat::
        //    return EFormatType::BC3;
        //case VkFormat::
        //    return EFormatType::BC4;
        //case VkFormat::
        //    return EFormatType::BC5;
        //case VkFormat::
        //    return EFormatType::BC6;
        //case VkFormat::
        return EFormatType::BC7;
    case VkFormat::VK_FORMAT_D16_UNORM:
        return EFormatType::D_16;
    case VkFormat::VK_FORMAT_D16_UNORM_S8_UINT:
        return EFormatType::D_16_S8;
    case VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32:
        return EFormatType::D_24;
    case VkFormat::VK_FORMAT_D24_UNORM_S8_UINT:
        return EFormatType::D_24_S8;
    case VkFormat::VK_FORMAT_D32_SFLOAT:
        return EFormatType::D_32;
    case VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT:
        return EFormatType::D_32_S8;
    default:
        CASSERT(false);
    }

    return EFormatType::Undefined;
}

inline VkFormat ConvertFormatType(EFormatType Format)
{
    switch (Format)
    {
    case EFormatType::Undefined:
        return VkFormat::VK_FORMAT_UNDEFINED;
    case EFormatType::RGBA8_UNORM:
        return VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
    case EFormatType::RGBA8_SRGB:
        return VkFormat::VK_FORMAT_B8G8R8A8_SRGB;
    case EFormatType::RG8_UNORM:
        return VkFormat::VK_FORMAT_R8G8_UNORM;
    case EFormatType::R8_UNORM:
        return VkFormat::VK_FORMAT_R8_UNORM;
    case EFormatType::R10G10B10A2_UNORM:
        return VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case EFormatType::R11G11B10_Float:
        return VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case EFormatType::RGBA16_Float:
        return VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;
    case EFormatType::RG16_Float:
        return VkFormat::VK_FORMAT_R16G16_SFLOAT;
    case EFormatType::R16_Float:
        return VkFormat::VK_FORMAT_R16_SFLOAT;
    case EFormatType::R16_UINT:
        return VkFormat::VK_FORMAT_R16G16_UINT;
    case EFormatType::RGBA32_Float:
        return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
    case EFormatType::RGB32_Float:
        return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
    case EFormatType::RG32_Float:
        return VkFormat::VK_FORMAT_R32G32_SFLOAT;
    case EFormatType::R32_Float:
        return VkFormat::VK_FORMAT_R32_SFLOAT;
    case EFormatType::R32_UINT:
        return VkFormat::VK_FORMAT_R32_UINT;
    case EFormatType::BC1_SRGB:
        return VkFormat::VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case EFormatType::BC2_SRGB:
        return VkFormat::VK_FORMAT_BC2_SRGB_BLOCK;
    case EFormatType::BC3:
        break;
    case EFormatType::BC4:
        break;
    case EFormatType::BC5:
        break;
    case EFormatType::BC6:
        break;
    case EFormatType::BC7:
        break;
    case EFormatType::D_16:
        return VkFormat::VK_FORMAT_D16_UNORM;
    case EFormatType::D_16_S8:
        return VkFormat::VK_FORMAT_D16_UNORM_S8_UINT;
    case EFormatType::D_24:
        return VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32;
    case EFormatType::D_24_S8:
        return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;
    case EFormatType::D_32:
        return VkFormat::VK_FORMAT_D32_SFLOAT;
    case EFormatType::D_32_S8:
        return VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;
    default:
        CASSERT(false);
    }

    return VkFormat::VK_FORMAT_UNDEFINED;
}

inline VkImageLayout ConvertLayoutType(EImageLayoutType Layout)
{
    switch (Layout)
    {
    case EImageLayoutType::Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case EImageLayoutType::Default:
        return VK_IMAGE_LAYOUT_GENERAL;
    case EImageLayoutType::ColorAttachment:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case EImageLayoutType::ShaderReadOnly:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case EImageLayoutType::TransferSrc:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case EImageLayoutType::TransferDst:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case EImageLayoutType::ReadOnly:
        return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    case EImageLayoutType::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    CASSERT(false);
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

inline VkImageTiling ConvertTilingType(ETilingType Tiling)
{
    switch (Tiling)
    {
    case ETilingType::Linear:
        return VK_IMAGE_TILING_LINEAR;
    case ETilingType::Optimal:
        return VK_IMAGE_TILING_OPTIMAL;
    }

    CASSERT(false);
    return VK_IMAGE_TILING_OPTIMAL;
}

inline VkSampleCountFlagBits ConvertSamlpeCountType(uint16 SamplesCount)
{
    switch (SamplesCount)
    {
    case 1:
        return VK_SAMPLE_COUNT_1_BIT;
    case 2:
        return VK_SAMPLE_COUNT_2_BIT;
    case 4:
        return VK_SAMPLE_COUNT_4_BIT;
    case 8:
        return VK_SAMPLE_COUNT_8_BIT;
    case 16:
        return VK_SAMPLE_COUNT_16_BIT;
    }

    CASSERT(false);
    return VK_SAMPLE_COUNT_1_BIT;
}
inline VkImageType ConvertImageType(EImageType ImageType)
{
    switch (ImageType)
    {
    case EImageType::Type1D:
        return VK_IMAGE_TYPE_1D;

    case EImageType::Type2D:
        return VK_IMAGE_TYPE_2D;

    case EImageType::Type3D:
        return VK_IMAGE_TYPE_3D;
    }

    CASSERT(false);
    return VK_IMAGE_TYPE_3D;
}

inline VkImageUsageFlags ConvertTextureUsageType(EImageUsageType Usage)
{
    VkImageUsageFlags Flags = 0;

    uint32 UsageBits = (uint32)Usage;

    if (UsageBits & (uint32)EImageUsageType::TransferScr)
        Flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (UsageBits & (uint32)EImageUsageType::TransferDst)
        Flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (UsageBits & (uint32)EImageUsageType::Sampled)
        Flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (UsageBits & (uint32)EImageUsageType::Storage)
        Flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (UsageBits & (uint32)EImageUsageType::ColorAttachment)
        Flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (UsageBits & (uint32)EImageUsageType::DepthStencil)
        Flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (UsageBits & (uint32)EImageUsageType::ShaderResourceView)
        Flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    return Flags;
}

inline VkBufferUsageFlags ConvertBufferUsageType(EBufferUsageType Usage)
{
    VkBufferUsageFlags Flags = 0;

    uint32 UsageBits = (uint32)Usage;

    if (UsageBits & (uint32)EBufferUsageType::TransferScr)
        Flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (UsageBits & (uint32)EBufferUsageType::TransferDst)
        Flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (UsageBits & (uint32)EBufferUsageType::Uniform)
        Flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (UsageBits & (uint32)EBufferUsageType::Storage)
        Flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (UsageBits & (uint32)EBufferUsageType::Index)
        Flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (UsageBits & (uint32)EBufferUsageType::Vertex)
        Flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (UsageBits & (uint32)EBufferUsageType::Indirect)
        Flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    return Flags;
}

inline VkImageAspectFlags ConvertImageAspectType(EImageAspectType Access)
{
    VkImageAspectFlags Flags = 0;

    uint32 AccessBits = (uint32)Access;

    if (AccessBits & (uint32)EImageAspectType::Color)
        Flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    if (AccessBits & (uint32)EImageAspectType::Depth)
        Flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if (AccessBits & (uint32)EImageAspectType::Stencil)
        Flags |= VK_IMAGE_ASPECT_STENCIL_BIT;

    if (AccessBits & (uint32)EImageAspectType::Metadata)
        Flags |= VK_IMAGE_ASPECT_METADATA_BIT;

    return Flags;
}

inline VkAttachmentLoadOp ConvertLoadOpType(ERenderTargetLoadOp LoadOp)
{
    switch (LoadOp)
    {
    case Cyclone::Render::ERenderTargetLoadOp::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case Cyclone::Render::ERenderTargetLoadOp::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case Cyclone::Render::ERenderTargetLoadOp::DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    CASSERT(false);
    return VK_ATTACHMENT_LOAD_OP_LOAD;
}

inline VkAttachmentStoreOp ConvertStoreOpType(ERenderTargetStoreOp StoreOp)
{
    switch (StoreOp)
    {
    case Cyclone::Render::ERenderTargetStoreOp::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case Cyclone::Render::ERenderTargetStoreOp::DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    CASSERT(false);
    return VK_ATTACHMENT_STORE_OP_STORE;
}

inline VkVertexInputRate ConvertVertexBindingRate(EVertexBindingRate Rate)
{
    switch (Rate)
    {
    case EVertexBindingRate::Vertex:
        return VK_VERTEX_INPUT_RATE_VERTEX;
    case EVertexBindingRate::Instance:
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    }

    CASSERT(false);
    return VK_VERTEX_INPUT_RATE_VERTEX;
}

inline VkPrimitiveTopology ConvertVertexBindingRate(PrimitiveTopologyType Rate)
{
    switch (Rate)
    {
    case PrimitiveTopologyType::TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    CASSERT(false);
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

inline VkCullModeFlags ConvertCullMode(ECullMode Cull)
{
    switch (Cull)
    {
    case ECullMode::Disable:
        return VK_CULL_MODE_NONE;

    case ECullMode::Back:
        return VK_CULL_MODE_BACK_BIT;

    case ECullMode::Front:
        return VK_CULL_MODE_FRONT_BIT;
    }

    CASSERT(false);
    return VK_CULL_MODE_BACK_BIT;
}

inline VkPolygonMode ConvertPolygonFillMode(EPolygonFillMode FillMode)
{
    switch (FillMode)
    {
    case EPolygonFillMode::Fill:
        return VK_POLYGON_MODE_FILL;
    case EPolygonFillMode::Line:
        return VK_POLYGON_MODE_LINE;
    case EPolygonFillMode::Point:
        return VK_POLYGON_MODE_POINT;
    }

    CASSERT(false);
    return VK_POLYGON_MODE_FILL;
}

inline VkCompareOp ConvertOperationMode(ECompareOp Op)
{
    switch (Op)
    {
    case Cyclone::Render::ECompareOp::Never:
        return VK_COMPARE_OP_NEVER;
    case Cyclone::Render::ECompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case Cyclone::Render::ECompareOp::Greater:
        return VK_COMPARE_OP_GREATER;
    case Cyclone::Render::ECompareOp::Equal:
        return VK_COMPARE_OP_EQUAL;
    case Cyclone::Render::ECompareOp::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case Cyclone::Render::ECompareOp::LessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case Cyclone::Render::ECompareOp::GreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case Cyclone::Render::ECompareOp::Always:
        return VK_COMPARE_OP_ALWAYS;
    }
    CASSERT(false);
    return VK_COMPARE_OP_GREATER;
}

} //namespace Cyclone::Render
