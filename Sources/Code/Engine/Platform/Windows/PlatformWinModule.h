#pragma once

#include "Engine/Framework/IModule.h"
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

namespace Cyclone
{

class PLATFORMWIN_API PlatformWinModule : public IModule
{
public:
    virtual C_STATUS OnRegister() override;
    virtual C_STATUS OnUnRegister() override;
};

} // namespace Cyclone
