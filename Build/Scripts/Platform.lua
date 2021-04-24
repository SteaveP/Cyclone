include "Common.lua"
include "Engine.lua"

function AddPlatformsDependency()
	links "PlatformWin"
end

function IncludeCyclonePlatform()	
	-- #todo add conditions project as well for other platforms
	project "PlatformWin"
		SetupDefaultProjectState("PlatformWin", "Library")
		language "C++"

		files { EnginePath("Platform/Windows/**.h"), EnginePath("Platform/Windows/**.cpp") }

		vpaths {
			["Windows/*"] = { EnginePath("Platform/Windows/**.h"), EnginePath("Platform/Windows/**.cpp") }
		}

		AddEngineDependency()
end