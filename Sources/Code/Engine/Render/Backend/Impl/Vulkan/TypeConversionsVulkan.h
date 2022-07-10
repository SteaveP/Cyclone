#pragma once

#include "Engine/Render/CoreRender.h"

namespace Cyclone::Render
{

inline VkFormat GFormatToVulkan[(uint32)EFormatType::Count] = 
{
    VkFormat::VK_FORMAT_UNDEFINED,                 // Undefined
    VkFormat::VK_FORMAT_R8G8B8A8_UNORM,            // RGBA8_UNORM
    VkFormat::VK_FORMAT_R8G8B8A8_SRGB,             // RGBA8_SRGB
    VkFormat::VK_FORMAT_B8G8R8A8_UNORM,            // BGRA8_UNORM
    VkFormat::VK_FORMAT_B8G8R8A8_SRGB,             // BGRA8_SRGB
    VkFormat::VK_FORMAT_R8G8_UNORM,                // RG8_UNORM
    VkFormat::VK_FORMAT_R8_UNORM,                  // R8_UNORM
    VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32,  // R10G10B10A2_UNORM
    VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32,   // R11G11B10_Float
    VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT,       // RGBA16_Float
    VkFormat::VK_FORMAT_R16G16_SFLOAT,             // RG16_Float
    VkFormat::VK_FORMAT_R16_SFLOAT,                // R16_Float
    VkFormat::VK_FORMAT_R16_UINT,                  // R16_UINT
    VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT,       // RGBA32_Float
    VkFormat::VK_FORMAT_R32G32B32_SFLOAT,          // RGB32_Float
    VkFormat::VK_FORMAT_R32G32_SFLOAT,             // RG32_Float
    VkFormat::VK_FORMAT_R32_SFLOAT,                // R32_Float
    VkFormat::VK_FORMAT_R32_UINT,                  // R32_UINT
    // BC
    VkFormat::VK_FORMAT_BC1_RGBA_SRGB_BLOCK,       // BC1_SRGB
    VkFormat::VK_FORMAT_BC2_SRGB_BLOCK,            // BC2_SRGB
    VkFormat::VK_FORMAT_BC3_SRGB_BLOCK,            // BC3
    VkFormat::VK_FORMAT_BC4_UNORM_BLOCK,           // BC4
    VkFormat::VK_FORMAT_BC5_UNORM_BLOCK,           // BC5
    VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK,         // BC6
    VkFormat::VK_FORMAT_BC7_SRGB_BLOCK,            // BC7
    // Depth
    VkFormat::VK_FORMAT_D16_UNORM,                 // D_16
    VkFormat::VK_FORMAT_D16_UNORM_S8_UINT,         // D_16_S8
    VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32,       // D_24
    VkFormat::VK_FORMAT_D24_UNORM_S8_UINT,         // D_24_S8
    VkFormat::VK_FORMAT_D32_SFLOAT,                // D_32
    VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT,        // D_32_S8
};

inline EFormatType ConvertFormatType(VkFormat Format)
{
    switch (Format)
    {
    case VkFormat::VK_FORMAT_UNDEFINED:
        return EFormatType::Undefined;
    case VkFormat::VK_FORMAT_R8G8B8A8_UNORM:
        return EFormatType::RGBA8_UNORM;
    case VkFormat::VK_FORMAT_B8G8R8A8_SRGB:
        return EFormatType::BGRA8_SRGB;
    case VkFormat::VK_FORMAT_R8G8B8A8_SRGB:
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
    CASSERT((uint32)Format < (uint32)EFormatType::Count);
    return GFormatToVulkan[(uint32)Format];
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
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // #todo_vk should be VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL (new in VK 1.2 with Syncrhonization2)
    case EImageLayoutType::DepthStencil:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case EImageLayoutType::DepthStencilReadOnly:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
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
inline VkImageType ConvertTextureType(ETextureType TextureType)
{
    switch (TextureType)
    {
    case ETextureType::Type1D:
        return VK_IMAGE_TYPE_1D;

    case ETextureType::Type2D:
        return VK_IMAGE_TYPE_2D;

    case ETextureType::Type3D:
        return VK_IMAGE_TYPE_3D;
    }

    CASSERT(false);
    return VK_IMAGE_TYPE_2D;
}

inline VkImageViewType ConvertTextureViewType(ETextureViewType ImageViewType)
{
    switch (ImageViewType)
    {
    case ETextureViewType::Type1D:
        return VK_IMAGE_VIEW_TYPE_1D;

    case ETextureViewType::Type2D:
        return VK_IMAGE_VIEW_TYPE_2D;

    case ETextureViewType::Type3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    }

    CASSERT(false);
    return VK_IMAGE_VIEW_TYPE_2D;
}
inline VkImageUsageFlags ConvertTextureUsageType(EImageUsageType Usage)
{
    VkImageUsageFlags Flags = 0;

    if (Usage & EImageUsageType::TransferSrc)
        Flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (Usage & EImageUsageType::TransferDst)
        Flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (Usage & EImageUsageType::Sampled)
        Flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (Usage & EImageUsageType::Storage)
        Flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (Usage & EImageUsageType::ColorAttachment)
        Flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (Usage & (EImageUsageType::DepthStencil | EImageUsageType::DepthStencilRead))
        Flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (Usage & (EImageUsageType::ShaderResourceView | EImageUsageType::DepthStencilRead))
        Flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (Usage & EImageUsageType::Present) // #todo_vk maybe  need to refactor this
        Flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    return Flags;
}

inline VkBufferUsageFlags ConvertBufferUsageType(EBufferUsageType Usage)
{
    VkBufferUsageFlags Flags = 0;

    if (Usage & EBufferUsageType::TransferSrc)
        Flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (Usage & EBufferUsageType::TransferDst)
        Flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (Usage & EBufferUsageType::Uniform)
        Flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (Usage & EBufferUsageType::Storage)
        Flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (Usage & EBufferUsageType::Index)
        Flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (Usage & EBufferUsageType::Vertex)
        Flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (Usage & EBufferUsageType::Indirect)
        Flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    if (Usage & EBufferUsageType::UniformTexel)
        Flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    if (Usage & EBufferUsageType::StorageTexel)
        Flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

    return Flags;
}

inline VkImageAspectFlags ConvertImageAspectType(EImageAspectType Access)
{
    VkImageAspectFlags Flags = 0;

    if (Access & EImageAspectType::Color)
        Flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    if (Access & EImageAspectType::Depth)
        Flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if (Access & EImageAspectType::Stencil)
        Flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    if (Access & EImageAspectType::Metadata)
        Flags |= VK_IMAGE_ASPECT_METADATA_BIT;

    return Flags;
}

inline VkAttachmentLoadOp ConvertLoadOpType(ERenderTargetLoadOp LoadOp)
{
    switch (LoadOp)
    {
    case ERenderTargetLoadOp::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case ERenderTargetLoadOp::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case ERenderTargetLoadOp::DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    CASSERT(false);
    return VK_ATTACHMENT_LOAD_OP_LOAD;
}

inline VkAttachmentStoreOp ConvertStoreOpType(ERenderTargetStoreOp StoreOp)
{
    switch (StoreOp)
    {
    case ERenderTargetStoreOp::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case ERenderTargetStoreOp::DontCare:
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
    case ECompareOp::Never:
        return VK_COMPARE_OP_NEVER;
    case ECompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case ECompareOp::Greater:
        return VK_COMPARE_OP_GREATER;
    case ECompareOp::Equal:
        return VK_COMPARE_OP_EQUAL;
    case ECompareOp::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case ECompareOp::LessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ECompareOp::GreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ECompareOp::Always:
        return VK_COMPARE_OP_ALWAYS;
    }
    CASSERT(false);
    return VK_COMPARE_OP_GREATER;
}

inline VkDescriptorType ConvertDescriptorType(EDescriptorType Type)
{
    switch (Type)
    {
    case EDescriptorType::Sampler:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case EDescriptorType::Buffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case EDescriptorType::BufferUAV:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case EDescriptorType::Texture:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case EDescriptorType::TextureUAV:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }

    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

inline VkAccessFlagBits2 ConvertMemoryAccessMask(EMemoryAccessMask Mask)
{
    VkAccessFlagBits2 Result = 0;

    if (Mask & EMemoryAccessMask::ShaderRead)
        Result |= VK_ACCESS_2_SHADER_READ_BIT;
    if (Mask & EMemoryAccessMask::ShaderWrite)
        Result |= VK_ACCESS_2_SHADER_WRITE_BIT;
    if (Mask & EMemoryAccessMask::ColorAttachmentRead)
        Result |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    if (Mask & EMemoryAccessMask::ColorAttachmentWrite)
        Result |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    if (Mask & EMemoryAccessMask::DepthStencilRead)
        Result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if (Mask & EMemoryAccessMask::DepthStencilWrite)
        Result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if (Mask & EMemoryAccessMask::TransferRead)
        Result |= VK_ACCESS_2_TRANSFER_READ_BIT;
    if (Mask & EMemoryAccessMask::TransferWrite)
        Result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;

    CASSERT(((uint64)Mask != 0) == (Result != 0));

    return Result;
}

inline VkPipelineStageFlagBits2 ConvertExecutionStageMask(EExecutionStageMask Mask)
{
    VkPipelineStageFlagBits2 Result = 0;

    if (Mask & EExecutionStageMask::VertexShader)
        Result |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    if (Mask & EExecutionStageMask::PixelShader)
        Result |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    if (Mask & EExecutionStageMask::ComputeShader)
        Result |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    if (Mask & EExecutionStageMask::ColorAttachmentOutput)
        Result |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (Mask & EExecutionStageMask::DepthStencil)
        Result |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;

    if (Mask & EExecutionStageMask::Transfer)
        Result |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;

    if (Mask & EExecutionStageMask::AllGraphicsCommands)
        Result |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    if (Mask & EExecutionStageMask::AllCommands)
        Result |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    CASSERT(((uint64)Mask != 0) == (Result != 0));

    return Result;
}

inline VkPipelineBindPoint ConvertPipelineType(PipelineType Type)
{
    switch (Type)
    {
    case PipelineType::Graphics:
        return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
    case PipelineType::Compute:
        return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
    case PipelineType::Raytracing:
        return VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    }

    CASSERT(false);
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

} //namespace Cyclone::Render
