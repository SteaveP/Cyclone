#pragma once

#ifdef DYNAMIC_LIB
#ifdef DYNAMIC_LIB_VULKAN
#define RENDER_BACKEND_VK_API DLL_EXPORT
#else
#define RENDER_BACKEND_VK_API DLL_IMPORT
#endif
#else
#define RENDER_BACKEND_VK_API
#endif
