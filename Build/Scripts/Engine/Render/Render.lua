include "../Engine.lua"

function SetupRenderBackendProject(BackendName)
	filter { "platforms:not *"..BackendName}
		if BackendName == "DX12" then
			removeplatforms("*Vulkan")
		else
			removeplatforms("*DX12")
		end
	filter {}

	filter { "platforms:Win*Vulkan"}
		defines { "VK_USE_PLATFORM_WIN32_KHR" }
	filter {}
end

function SetupExternalRenderBackendProject(RHIName)
	return SetupRenderBackendProject(RHIName)
end

include "RenderBackendVulkan.lua"
include "RenderBackendDX12.lua"

function AddRendersDependency()
	links { "Vulkan"}
	links { "DX12"}
end

function IncludeEngineRenderBackends()
	IncludeEngineRenderPlatformVulkan()
	IncludeEngineRenderPlatformDX12()
end

function IncludeEngineRender()	
	PushGroup "Backend"
		IncludeEngineRenderBackends()
	PopGroup()
end
