#pragma once

#ifdef DYNAMIC_LIB
	#ifdef DYNAMIC_LIB_PLATFORMWIN
		#define PLATFORM_WIN_API DLL_EXPORT
	#else
		#define PLATFORM_WIN_API DLL_IMPORT
	#endif
#else
	#define PLATFORM_WIN_API
	#define PLATFORM_WIN_API
#endif
