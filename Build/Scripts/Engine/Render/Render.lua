include "../Engine.lua"

function AddRendersDependency()
	links { "RenderBackendVulkan"}
end

local function GetVulkanSDKPath()
	-- for example "C:/VulkanSDK/1.2.141.2/"
	local VulkanSDKPath = os.getenv("VULKAN_SDK")
	return VulkanSDKPath
end

local function RelativeVulkanSDKPath(dirname)
	return path.join(GetVulkanSDKPath(), dirname)
end

local function SetupRenderBackendProject(BackendName)
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

function IncludeEngineRenderPlatformVulkan()
	local BackendName = "Vulkan";

	-- #todo also add min version checking
	if GetVulkanSDKPath() == "" then
		print("Vulkan SDK not found! Vulkan Backend will not be generated")
		return
	end

	PushGroup "Vulkan"
		project "RenderBackendVulkan"
			SetupDefaultProjectState("RenderBackendVulkan", "Library")
			SetupRenderBackendProject(BackendName)
			
			files {
				EnginePath("Render/Backend/Vulkan/**.h"),
				EnginePath("Render/Backend/Vulkan/**.cpp"),
			}
			
			vpaths {
				["Code/*"] = {EnginePath("Render/Backend/Vulkan/**")},
			}

			includedirs { EnginePath("Render/Backend/Vulkan") }
			includedirs { RelativeVulkanSDKPath("Include") }
			libdirs { RelativeVulkanSDKPath("Lib") }
			links { "vulkan-1" }

			AddEngineDependencyInternal()

			-- ImGui Renderer
			IncludeImGuiReference()
			files {
				SourcesPath("ThirdParty/ImGui/backends/imgui_impl_vulkan.h"),
				SourcesPath("ThirdParty/ImGui/backends/imgui_impl_vulkan.cpp")
			}
			
			filter { "toolset:msc*" }
				files { SourcesPath("ThirdParty/ImGui/misc/natvis/*.natvis")}
			filter {}

			vpaths { ["Code/UI/ImGui/*"] = { SourcesPath("ThirdParty/ImGui**") } }
			
			-- Vulkan Memory Allocator
			files { SourcesPath("ThirdParty/Render/BackendVulkan/VulkanMemoryAllocator/include/vk_mem_alloc.h") }
			filter { "toolset:msc*" }
				files { SourcesPath("ThirdParty/Render/BackendVulkan/VulkanMemoryAllocator/src/**.natvis")}
			filter {}			
			includedirs { SourcesPath("ThirdParty/Render/BackendVulkan/VulkanMemoryAllocator/include/") }
			
			vpaths { ["Code/MemoryAllocator/ThirdParty/*"] = { SourcesPath("ThirdParty/Render/BackendVulkan/VulkanMemoryAllocator/**") } }
	PopGroup()
end

function IncludeEngineRenderBackends()
	IncludeEngineRenderPlatformVulkan()
end

function IncludeEngineRender()	
	PushGroup "Backend"
		IncludeEngineRenderBackends()
	PopGroup()
end
