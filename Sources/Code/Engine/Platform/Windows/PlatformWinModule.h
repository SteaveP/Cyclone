#include "Engine/Core/Helpers.h"

#ifdef DYNAMIC_LIB
	#ifdef DYNAMIC_LIB_PLATFORMWIN
		#define PLATFORMWIN_API DLL_EXPORT
	#else
		#define PLATFORMWIN_API DLL_IMPORT
	#endif
#else
	#define PLATFORMWIN_API
	#define PLATFORMWIN_API
#endif