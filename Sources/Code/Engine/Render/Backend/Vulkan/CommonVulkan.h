#pragma once

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES // for alignment that match Vulkan's shaders
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//
//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/gtx/hash.hpp>

////#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
////#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

#ifdef PLATFORM_WIN64
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <map>
#include <unordered_map>
#include <set>
#include <optional>
#include <array>
#include <chrono>

#include "Engine/Core/Types.h"
#include "Engine/Core/Math.h"
#include "Engine/Core/Helpers.h"

#include "Engine/Render/Common.h"

#include "RenderBackendVulkanModuleDefines.h"

#define C_ASSERT_VK_SUCCEEDED(x) CASSERT((x) == VkResult::VK_SUCCESS)
#define C_ASSERT_VK_SUCCEEDED_RET(x, ret) C_ASSERT_RETURN_VAL((x) == VkResult::VK_SUCCESS, ret)

namespace Cyclone::Render
{

class RenderBackendVulkan;

class GlobalContextVulkan;
class WindowContextVulkan;

class CommandQueueVulkan;
class CommandBufferVulkan;

using VulkanHandle = uint32;
const VulkanHandle VulkanNullHandle = static_cast<uint32>(-1);

struct DeviceHandle
{
    static const uint16 InvalidHandle = static_cast<uint16>(-1);

    uint16 PhysicalDeviceHandle = InvalidHandle;
    uint16 LogicalDeviceHandle = InvalidHandle;

    bool IsPhysicalDeviceHandleValid() const { return PhysicalDeviceHandle != InvalidHandle; }
    bool IsLogicalDeviceHandleValid() const { return LogicalDeviceHandle != InvalidHandle; }
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
    case VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT:
        return EFormatType::RGBA32_Float;
    case VkFormat::VK_FORMAT_R32G32B32_SFLOAT:
        return EFormatType::RGB32_Float;
    case VkFormat::VK_FORMAT_R32G32_SFLOAT:
        return EFormatType::RG32_Float;
    case VkFormat::VK_FORMAT_R32_SFLOAT:
        return EFormatType::R32_Float;
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
    case EFormatType::RGBA32_Float:
        return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
    case EFormatType::RGB32_Float:
        return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
    case EFormatType::RG32_Float:
        return VkFormat::VK_FORMAT_R32G32_SFLOAT;
    case EFormatType::R32_Float:
        return VkFormat::VK_FORMAT_R32_SFLOAT;
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

} // namespace Cyclone::Render
