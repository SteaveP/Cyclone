// Need to include only internal common file that doesn't contain vk_mem_alloc.h include
// because of Unity builds issues

#include "Internal/CommonVulkanInternal.h"

#if ENABLE_VOLK_LOADER
// Load implementation
#include "../ThirdParty/Render/Vulkan/Volk/volk.c"
#endif

#define VMA_IMPLEMENTATION 1
#include <vk_mem_alloc.h>
