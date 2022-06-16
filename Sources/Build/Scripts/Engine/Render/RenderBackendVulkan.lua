local function GetVulkanSDKPath()
	-- for example "C:/VulkanSDK/1.2.141.2/"
	local VulkanSDKPath = os.getenv("VULKAN_SDK")
	return VulkanSDKPath
end

local function RelativeVulkanSDKPath(dirname)
	return path.join(GetVulkanSDKPath(), dirname)
end

function IncludeEngineRenderPlatformVulkan()
	local BackendName = "Vulkan";

	-- #todo also add min version checking
	if GetVulkanSDKPath() == "" then
		print("Vulkan SDK not found! Vulkan Backend will not be generated")
		return
	end

	PushGroup "Vulkan"
		project "Vulkan"
			SetupDefaultProjectState("Vulkan", "Library")
			SetupRenderBackendProject(BackendName)
			
			files {
				EnginePath("Render/Backend/Vulkan/**.h"),
				EnginePath("Render/Backend/Vulkan/**.cpp"),
			}
			
			removefiles {EnginePath("Render/Backend/Vulkan/Modules/**")}
			
			vpaths {
				["Code/*"] = {EnginePath("Render/Backend/Vulkan/**")},
			}

			includedirs { EnginePath("Render/Backend/Vulkan") }
			includedirs { RelativeVulkanSDKPath("Include") }
			libdirs { RelativeVulkanSDKPath("Lib") }
			links { "vulkan-1" } -- #todo make dynamic Vulkan Function Pointers loading (Volk lib, https://gpuopen.com/learn/reducing-vulkan-api-call-overhead/)

			AddEngineDependencyInternal()

			-- ImGui Renderer
			AddImGuiDependency()
			files {
				SourcesPath("ThirdParty/ImGui/backends/imgui_impl_vulkan.h"),
				SourcesPath("ThirdParty/ImGui/backends/imgui_impl_vulkan.cpp")
			}
			vpaths { ["Code/UI/ImGui/*"] = { SourcesPath("ThirdParty/ImGui**") } }
			
			
			filter { "toolset:msc*" }
				files { SourcesPath("ThirdParty/ImGui/misc/natvis/*.natvis")}
			filter {}

			-- Vulkan Memory Allocator
			files { SourcesPath("ThirdParty/Render/Vulkan/VulkanMemoryAllocator/include/vk_mem_alloc.h") }
			filter { "toolset:msc*" }
				files { SourcesPath("ThirdParty/Render/Vulkan/VulkanMemoryAllocator/src/**.natvis")}
			filter {}			
			includedirs { SourcesPath("ThirdParty/Render/Vulkan/VulkanMemoryAllocator/include/") }
			
			vpaths { ["Code/Internal/MemoryAllocator/ThirdParty/*"] = { SourcesPath("ThirdParty/Render/Vulkan/VulkanMemoryAllocator/**") } }
	PopGroup()
end
