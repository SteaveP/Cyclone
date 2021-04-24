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

#include <vulkan/vulkan.h>

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