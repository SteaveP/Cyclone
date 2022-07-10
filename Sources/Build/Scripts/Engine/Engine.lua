include "../Common.lua"
include "Platform.lua"
include "Render/Render.lua"

-- should be used for internal engine's libs like platforms or render backends
function AddEngineDependencyInternal()
	links "Engine"
	
	AddSPDLogDependency()
	AddOptickDependency()
	AddEnkiTSDependency()
	AddEnTTDependency()
	AddAsyncMulticastDelegateDependency()
	
	-- #todo Do this only if Engine was build as SharedLib (need to check Premake sources to see how to do it)
	-- #todo but better would be to move ImGUI to separate lib and link engine against it
	filter { "configurations:*DLL*" }
		defines { "IMGUI_API=__declspec(dllimport)" }		
		defines { "IMGUI_IMPL_API=" }
	filter {}
end

function AddImGuiDependency(WithLinks)
	includedirs {
		SourcesPath("ThirdParty/ImGui/"),
		SourcesPath("ThirdParty/ImGuiFileDialog/3rdparty/dirent/include"),
	}
end

function AddJSONDependency()	
	includedirs { SourcesPath("ThirdParty/Json/single_include") }
end

function AddSPDLogDependency()	
	includedirs { SourcesPath("ThirdParty/Spdlog/include") }
end

function AddOptickDependency()
	links "Profiling"
	includedirs {
		--EnginePath("Modules/Optick/**")
		SourcesPath("ThirdParty/Optick/src/")
	}
end

function AddEnkiTSDependency()
	-- #todo make as separate library?
	includedirs { SourcesPath("ThirdParty/EnkiTS/src/")}
	
	filter { "configurations:*DLL*" }
		defines { "ENKITS_DLL" }
	filter{}
end

function AddEnTTDependency()
	-- #todo 
end

function AddAsyncMulticastDelegateDependency()
	includedirs { SourcesPath("ThirdParty/AsyncMulticastDelegate")}	
end

-- shound be used for external projects like Editor that want to fully include the Engine
function AddEngineDependency()
	AddEngineDependencyInternal()	
	AddPlatformsDependency()
	AddRendersDependency()
	AddImGuiDependency()
	AddEnkiTSDependency()
	AddEnTTDependency()
	AddAsyncMulticastDelegateDependency()
end

-- #todo move to separate project and link to others
function IncludeImGui()
	AddImGuiDependency()

	filter "kind:SharedLib"
		defines { "IMGUI_API=__declspec(dllexport)" }
	filter {}
		
	filter { "toolset:msc*" }
		files { SourcesPath("ThirdParty/ImGui/misc/natvis/*.natvis")}
	filter {}

	files {
		-- imgui
		SourcesPath("ThirdParty/ImGui/imconfig.h"),
		SourcesPath("ThirdParty/ImGui/imgui.h"),
		SourcesPath("ThirdParty/ImGui/imgui_internal.h"),
		SourcesPath("ThirdParty/ImGui/imstb_rectpack.h"),
		SourcesPath("ThirdParty/ImGui/imstb_textedit.h"),
		SourcesPath("ThirdParty/ImGui/imstb_truetype.h"),
		SourcesPath("ThirdParty/ImGui/imgui.cpp"),
		--SourcesPath("ThirdParty/ImGui/imgui_demo.cpp"),
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

function IncludeOptick()
	project "Profiling"
		SetupDefaultProjectState("Profiling", "Library")
		language "C++"

		defines { "OPTICK_EXPORTS" }

		includedirs {
			--EnginePath("Modules/Optick/**")
			SourcesPath("ThirdParty/Optick/src/")
		}

		files {
			EnginePath("Modules/Profiling/**.cpp"),
			EnginePath("Modules/Profiling/**.h"),
			EnginePath("Modules/Profiling/**.inl"),

			SourcesPath("ThirdParty/Optick/src/optick_server.h"),
			SourcesPath("ThirdParty/Optick/src/optick_serialization.cpp"),
			SourcesPath("ThirdParty/Optick/src/optick_serialization.h"),
			SourcesPath("ThirdParty/Optick/src/optick_server.cpp"),
			SourcesPath("ThirdParty/Optick/src/optick_miniz.cpp"),
			SourcesPath("ThirdParty/Optick/src/optick_miniz.h"),
			SourcesPath("ThirdParty/Optick/src/optick_message.cpp"),
			SourcesPath("ThirdParty/Optick/src/optick_message.h"),
			SourcesPath("ThirdParty/Optick/src/optick_gpu.h"),
			--SourcesPath("ThirdParty/Optick/src/optick_gpu.vulkan.cpp"),
			SourcesPath("ThirdParty/Optick/src/optick_memory.h"),
			SourcesPath("ThirdParty/Optick/src/optick_core.win.h"),
			SourcesPath("ThirdParty/Optick/src/optick_gpu.cpp"),
			--SourcesPath("ThirdParty/Optick/src/optick_gpu.d3d12.cpp"),
			--SourcesPath("ThirdParty/Optick/src/optick_core.linux.h"),
			--SourcesPath("ThirdParty/Optick/src/optick_core.macos.h"),
			SourcesPath("ThirdParty/Optick/src/optick_core.platform.h"),
			SourcesPath("ThirdParty/Optick/src/optick_core.cpp"),
			SourcesPath("ThirdParty/Optick/src/optick_core.h"),
			--SourcesPath("ThirdParty/Optick/src/optick_core.freebsd.h"),
			SourcesPath("ThirdParty/Optick/src/optick_capi.h"),
			SourcesPath("ThirdParty/Optick/src/optick_common.h"),
			SourcesPath("ThirdParty/Optick/src/optick.h"),
			SourcesPath("ThirdParty/Optick/src/optick.config.h"),
			SourcesPath("ThirdParty/Optick/src/optick_capi.cpp"),
		}

		vpaths {
			["*"] = {SourcesPath("**.cpp"), SourcesPath("**.h"), SourcesPath("**.inl"), SourcesPath("**.natvis")}
		}		
end

function IncludeEnkiTS()
	-- #todo as separate lib?
	
	files {
		SourcesPath("ThirdParty/EnkiTS/src/TaskScheduler.h"),
		SourcesPath("ThirdParty/EnkiTS/src/TaskScheduler.cpp"),
		SourcesPath("ThirdParty/EnkiTS/src/LockLessMultiReadPipe.h"),
	}

	vpaths {
		["Utils/EnkiTS/**"] = {SourcesPath("ThirdParty/EnkiTS/src/**")}
	}

	filter { "configurations:*DLL*" }
		defines { "ENKITS_BUILD_DLL" }
	filter{}
end

function IncludeEnTT()
	-- #todo as separate lib?
end

function IncludeAsyncMulticastDelegate()
	files {
		SourcesPath("ThirdParty/AsyncMulticastDelegate/Delegate/**"),
		SourcesPath("ThirdParty/AsyncMulticastDelegate/Port/**"),
	}

	vpaths {
		["Utils/Delegate/AsyncMulticastDelegate/**"] = {SourcesPath("ThirdParty/AsyncMulticastDelegate/**")}
	}
end
	
function IncludeEngine()
	PushGroup "Engine"
		project "Engine"
			SetupDefaultProjectState("Engine", "Library")
			language "C++"

			files {EnginePath("**.cpp"), EnginePath("**.h"), EnginePath("**.inl")}

			vpaths {
				["*"] = {EnginePath("**.cpp"), EnginePath("**.h"), EnginePath("**.inl"), EnginePath("**.natvis")}
			}

			removefiles {EnginePath("Platform/Impl/**")}
			removefiles {EnginePath("Modules/**")}
			removefiles {EnginePath("Render/Backend/Impl/**")}

			AddSPDLogDependency()
			AddJSONDependency()
			AddOptickDependency()
			AddAsyncMulticastDelegateDependency()
			AddEnkiTSDependency()
			AddEnTTDependency()

			IncludeAsyncMulticastDelegate()
			IncludeImGui()
			IncludeEnkiTS()
			IncludeEnTT()

		PushGroup "EngineLibraries"
			PushGroup "Platform"
				IncludeEnginePlatforms()
			PopGroup()
			PushGroup "Render"
				IncludeEngineRender()
			PopGroup()
			PushGroup "Other"
				IncludeOptick()
			PopGroup()
		PopGroup()
	PopGroup()
end
