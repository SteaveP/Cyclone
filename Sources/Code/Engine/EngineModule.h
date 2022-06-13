#pragma once

#include "Engine/Core/Helpers.h"

#ifdef DYNAMIC_LIB
	#ifdef DYNAMIC_LIB_ENGINE
		#define ENGINE_API DLL_EXPORT
	#else
		#define ENGINE_API DLL_IMPORT
	#endif
#else
	#define ENGINE_API
#endif
