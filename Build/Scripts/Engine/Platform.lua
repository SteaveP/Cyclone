include "../Common.lua"
include "Engine.lua"

function AddPlatformsDependency()
	links "PlatformWin"
end

function IncludeEnginePlatforms()	
	-- #todo add conditions project as well for other platforms
	project "PlatformWin"
		SetupDefaultProjectState("PlatformWin", "Library")
		language "C++"

		files { EnginePath("Platform/Windows/**.h"), EnginePath("Platform/Windows/**.cpp") }

		vpaths {
			["Windows/*"] = { EnginePath("Platform/Windows/**.h"), EnginePath("Platform/Windows/**.cpp") }
		}

		includedirs { EnginePath("Platform/Windows") }

		-- ImGui platform integration
		AddImGuiDependency()
		files {
			SourcesPath("ThirdParty/ImGui/backends/imgui_impl_win32.h"),
			SourcesPath("ThirdParty/ImGui/backends/imgui_impl_win32.cpp")
		}
		
		filter { "toolset:msc*" }
			files { SourcesPath("ThirdParty/ImGui/misc/natvis/*.natvis")}
		filter {}

		vpaths { ["Code/UI/ImGui/*"] = { SourcesPath("ThirdParty/ImGui**") } }
		
		AddEngineDependencyInternal()
end