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

#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <optional>
#include <array>
#include <chrono>

#include "../RenderBackendVkModule.h"

#define C_ASSERT_VK_SUCCEEDED(x) CASSERT((x) == VkResult::VK_SUCCESS)
#define C_ASSERT_VK_SUCCEEDED_RET(x, ret) C_ASSERT_RETURN_VAL((x) == VkResult::VK_SUCCESS, ret)

namespace Cyclone::Render
{

class RenderBackendVulkan;

class GlobalContextVulkan;
class WindowContextVulkan;

const int MAX_FRAMES_IN_FLIGHT = 2;

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

//////////////////////////////////////////////////////////////////////////
struct vec2
{
    float x;
    float y;

    bool operator == (const vec2& other) const noexcept
    {
        return x == other.x && y == other.y;
    }
};
struct vec3
{
    float x;
    float y;
    float z;

    bool operator == (const vec3& other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct vec4
{
    float x;
    float y;
    float z;
    float w;

    bool operator == (const vec4& other) const noexcept
    {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }
};

struct Vertex
{
    vec3 pos;
    vec3 color;
    vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    bool operator == (const Vertex& other) const noexcept { return pos == other.pos && color == other.color && texCoord == other.texCoord; }
};

enum class CommandQueueType
{
    Graphics,
    AsyncCompute,
    Present,
    Count
};

} // namespace Cyclone::Render

