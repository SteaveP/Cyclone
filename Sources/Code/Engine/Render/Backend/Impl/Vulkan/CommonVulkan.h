#pragma once

#include "Internal/CommonVulkanInternal.h"
#include <vk_mem_alloc.h>

// Engine
#include "Engine/Render/CommonRender.h" // #todo_vk include only CoreRender.h

// This module
#include "RenderBackendVulkanModuleDefines.h"
#include "TypeConversionsVulkan.h"

#define C_ASSERT_VK_SUCCEEDED(x) CASSERT((x) == VkResult::VK_SUCCESS)
#define C_ASSERT_VK_SUCCEEDED_RET(x, ret) C_ASSERT_RETURN_VAL((x) == VkResult::VK_SUCCESS, ret)

#define GET_CONFIG_VULKAN() GET_CONFIG_RENDER()["Vulkan"]

namespace Cyclone::Render
{

// Features
extern uint32 GEnableBindless;
extern uint32 GEnableAsyncCompute;
extern uint32 GEnableDynamicRendering;

// Debug/Validation
extern uint32 GEnableValidation;
extern uint32 GEnableGPUValidation;
extern uint32 GEnableRenderDocVk;

class CRenderBackendVulkan;

class CDeviceManagerVulkan;
class CWindowContextVulkan;
struct CPhysicalDevice;
struct CDevice;

class CCommandQueueVulkan;
class CCommandBufferVulkan;

class CResourceVulkan;
class CResourceViewVulkan;

using CVulkanHandle = uint32;
const CVulkanHandle VulkanNullHandle = static_cast<uint32>(-1);

#define BACKEND_DOWNCAST(Data, BackendType) static_cast<BackendType*>(Data)
#define GET_BACKEND_IMPL(Data) BACKEND_DOWNCAST(Data, CRenderBackendVulkan)

#if ENABLE_DEBUG_RENDER_BACKEND
bool SetDebugNameVk(std::string_view Name, VkObjectType Type, uint64 Handle, const struct CDevice& Device);
#endif

} // namespace Cyclone::Render
