#pragma once

#include "Engine/Core/Helpers.h"

#ifdef DYNAMIC_LIB
	#ifdef DYNAMIC_LIB_RENDERBACKENDVULKAN
		#define RENDER_BACKEND_VK_API DLL_EXPORT
	#else
		#define RENDER_BACKEND_VK_API DLL_IMPORT
	#endif
#else
	#define RENDER_BACKEND_VK_API
	#define RENDER_BACKEND_VK_API
#endif
