include "Common.lua"
include "Platform.lua"

function AddEngineDependency()
	links "Engine"
end

function IncludeEngine()
	PushGroup "Engine"
		project "Engine"
			SetupDefaultProjectState("Engine", "Library")
			language "C++"

			files {EnginePath("**.cpp"), EnginePath("**.h")}

			vpaths {
				["*"] = {EnginePath("**.cpp"), EnginePath("**.h"), EnginePath("**.natvis")}
			}
			
			removefiles {EnginePath("Platform/**")}

			PushGroup "CycloneEngineLibraries"
				PushGroup "Platform"
					IncludeCyclonePlatform()
				PopGroup()
			PopGroup()
		PopGroup()
end
