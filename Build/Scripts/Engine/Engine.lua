include "../Common.lua"
include "Platform.lua"
include "Render/Render.lua"

-- should be used for internal engine's libs like platforms or render backends
function AddEngineDependencyInternal()
	links "Engine"
end

-- shound be used for external projects like Editor that want to fully include the Engine
function AddEngineDependency()
	AddEngineDependencyInternal()	
	AddPlatformsDependency()
	AddRendersDependency()
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
			removefiles {EnginePath("Render/Backend/**")}

			PushGroup "EngineLibraries"
				PushGroup "Platform"
					IncludeEnginePlatforms()
				PopGroup()
				PushGroup "Render"
					IncludeEngineRender()
				PopGroup()
			PopGroup()
		PopGroup()
end
