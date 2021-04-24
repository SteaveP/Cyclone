#pragma once

#include "Engine/Core/Helpers.h"
#include "Engine/Framework/IModule.h"

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

namespace Cyclone
{

class RENDER_BACKEND_VK_API RenderBackendVulkanModule : public IModule
{
public:
    virtual C_STATUS OnRegister() override;
    virtual C_STATUS OnUnRegister() override;
};

} // namespace Cyclone
