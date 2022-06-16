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

#include <map>
#include <chrono>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "Engine/Render/Common.h"

#include "RenderBackendVulkanModuleDefines.h"
#include "Internal/FormatConversions.h"

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

#define BACKEND_DOWNCAST(Data, BackendType) static_cast<BackendType*>(Data)
#define GET_BACKEND_IMPL(Data) BACKEND_DOWNCAST(Data, RenderBackendVulkan)

} // namespace Cyclone::Render
