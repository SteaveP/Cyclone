-- Vulkan Function Pointers loading used instead of static linking
-- (see Volk lib, https://gpuopen.com/learn/reducing-vulkan-api-call-overhead/)
bEnableVolk = true

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
				EnginePath("Render/Backend/Impl/Vulkan/**.h"),
				EnginePath("Render/Backend/Impl/Vulkan/**.cpp"),
			}
			
			removefiles {EnginePath("Render/Backend/Impl/Vulkan/Modules/**")}
			
			vpaths {
				["Code/*"] = {EnginePath("Render/Backend/Impl/Vulkan/**")},
			}

			includedirs { EnginePath("Render/Backend/Impl/Vulkan") }
			includedirs { RelativeVulkanSDKPath("Include") }
			libdirs { RelativeVulkanSDKPath("Lib") }

			if (bEnableVolk) then
				defines { 
					"ENABLE_VOLK_LOADER=1",
					"IMGUI_IMPL_VULKAN_NO_PROTOTYPES",
				}

				filter { "platforms:Win64*"}
					defines { "VK_USE_PLATFORM_WIN32_KHR=1" }
				filter{}
			else				
				links { "vulkan-1" }
			end

			AddEngineDependencyInternal()

			-- ImGui Renderer
			AddImGuiDependency()
			files {
				-- There are modification for vulkan backend thus use local copy
				-- patches of changes are stored nearby in the same folder
				EnginePath("Render/Backend/Impl/Vulkan/UI/**"),
				EnginePath("Render/Backend/Impl/Vulkan/UI/imgui_impl_vulkan.h"),
				EnginePath("Render/Backend/Impl/Vulkan/UI/imgui_impl_vulkan.cpp")
			}
			vpaths { ["Code/UI/ImGui/*"] = { SourcesPath("ThirdParty/ImGui**") } }
			
			
			filter { "toolset:msc*" }
				files { SourcesPath("ThirdParty/ImGui/misc/natvis/*.natvis")}
			filter {}

			filter { "files:**MemoryAllocatorVk.cpp" }
				IncludeInUnityBuild "Off"
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
