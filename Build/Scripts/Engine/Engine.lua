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
	IncludeImGuiReference()
end

function IncludeImGuiReference()
	includedirs {
		SourcesPath("ThirdParty/ImGui/"),
		SourcesPath("ThirdParty/ImGuiFileDialog/3rdparty/dirent/include"),
	}
end

function IncludeImGui()
	IncludeImGuiReference()

	filter { "toolset:msc*" }
		files { SourcesPath("ThirdParty/ImGui/misc/natvis/*.natvis")}
	filter {}

	files {
		-- intergration
		SourcesPath("ThirdpartyIntegration/ImGui/**.h"),
		SourcesPath("ThirdpartyIntegration/ImGui/**.cpp"),
		-- imgui
		SourcesPath("ThirdParty/ImGui/imconfig.h"),
		SourcesPath("ThirdParty/ImGui/imgui.h"),
		SourcesPath("ThirdParty/ImGui/imgui_internal.h"),
		SourcesPath("ThirdParty/ImGui/imstb_rectpack.h"),
		SourcesPath("ThirdParty/ImGui/imstb_textedit.h"),
		SourcesPath("ThirdParty/ImGui/imstb_truetype.h"),
		SourcesPath("ThirdParty/ImGui/imgui.cpp"),
		SourcesPath("ThirdParty/ImGui/imgui_demo.cpp"),
		SourcesPath("ThirdParty/ImGui/imgui_draw.cpp"),
		SourcesPath("ThirdParty/ImGui/imgui_tables.cpp"),
		SourcesPath("ThirdParty/ImGui/imgui_widgets.cpp"),
		-- filedialog
		SourcesPath("ThirdParty/ImGuiFileDialog/ImGuiFileDialogConfig.h"),
		SourcesPath("ThirdParty/ImGuiFileDialog/ImGuiFileDialog.h"),
		SourcesPath("ThirdParty/ImGuiFileDialog/ImGuiFileDialog.cpp"),
	}

	vpaths {
		["UI/ImGui/Source/ImGui/**"] = { SourcesPath("ThirdParty/ImGui/**") }
	}	
	vpaths {
		["UI/ImGui/Source/ImGuiFileDialog**"] = { SourcesPath("ThirdParty/ImGuiFileDialog/**") }
	}
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

			IncludeImGui()

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
