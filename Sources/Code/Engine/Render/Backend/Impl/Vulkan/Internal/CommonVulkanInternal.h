#pragma once

// #todo_vk
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

#if ENABLE_VOLK_LOADER
    #define VK_NO_PROTOTYPES
    #include "../ThirdParty/Render/Vulkan/Volk/volk.h"
    #define VK_CALL(LogicDevice, Func) (LogicDevice).DeviceTable.##Func
#else
    #include <vulkan/vulkan.h>
    #define VK_CALL(LogicDevice, Func) Func
#endif
