local function AddD3D12MemoryAllocator(bHeaderOnly)
	-- D3D12 Memory Allocator
	if not bHeaderOnly then
		files {
			SourcesPath("Thirdparty/Render/DX12/D3D12MemoryAllocator/include/D3D12MemAlloc.h"),
			SourcesPath("Thirdparty/Render/DX12/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp"),
		}
	end
	includedirs { SourcesPath("Thirdparty/Render/DX12/D3D12MemoryAllocator/include") }
end

function IncludeEngineRenderPlatformDX12()
	local BackendName = "DX12";

	PushGroup "DX12"
		project "DX12"
			SetupDefaultProjectState("DX12", "Library")
			SetupRenderBackendProject(BackendName)
			
			AddEngineDependencyInternal()

			files {
				EnginePath("Render/Backend/DX12/**.h"),
				EnginePath("Render/Backend/DX12/**.cpp"),
				EnginePath("Render/Backend/DX12/Raytracing/*.h"),
				EnginePath("Render/Backend/DX12/Raytracing/*.cpp"),
			}

			vpaths {
				["Code/*"] 		 = {EnginePath("Render/Backend/DX12/**")},
				["Thirdparty/*"] = {SourcesPath("Thirdparty/Render/DX12/**")},
			}
	
			filter { "toolset:msc*" }
				files { SourcesPath("Thirdparty/Render/DX12/D3D12MemoryAllocator/**.natvis")}
			filter {}
			
			
			-- ImGui Renderer
			AddImGuiDependency()
			files {
				SourcesPath("ThirdParty/ImGui/backends/imgui_impl_dx12.h"),
				SourcesPath("ThirdParty/ImGui/backends/imgui_impl_dx12.cpp")
			}	
			vpaths { ["Code/UI/ImGui/*"] = { SourcesPath("ThirdParty/ImGui**") } }		

			AddD3D12MemoryAllocator(false)

			-- #TODO refactor links
			links { "d3d12", "dxgi", "dxguid" }

			
			-- links {"DirectXTK12", "DX12ShaderCompiler", "PIX" }

	
		PushGroup "Utils"
			project "DX12ShaderCompiler"
				SetupDefaultProjectState("DX12ShaderCompiler", "Library")
				SetupRenderBackendProject(BackendName)
				
				files {
					EnginePath("Render/Backend/DX12/DX12ShaderCompiler/**.h"),
					EnginePath("Render/Backend/DX12/DX12ShaderCompiler/**.cpp"),
				}

				links { "dxcompiler.lib" }
				
			externalproject "DirectXTK12"
				-- note that for external project location is no necessary
				SetupDefaultProjectState("DirectXTK12", "External")
				SetupExternalRenderBackendProject(BackendName)

				filename "DirectXTK_Desktop_2019_Win10"
				location (SourcesPath("Thirdparty/Render/DX12/DirectXTK12"))

				-- uuid, kind and language are mandatory
				uuid "3E0E8608-CD9B-4C76-AF33-29CA38F2C9F0"
				kind "StaticLib"
				language "C++"
				
				-- https://github.com/premake/premake-core/issues/1243
				-- inlude only DX12 platform
				configmap {
					["Win64_DX12"] = "x64",
					["Debug*"] = "Debug",
					["DebugOpt*"] = "Debug",
					["Profile*"] = "Release",
					["Release*"] = "Release"
				}
			
			externalproject "DirectXTex"
				-- note that for external project location is no necessary
				SetupDefaultProjectState("DirectXTex", "External")
				SetupExternalRenderBackendProject(BackendName)

				filename "DirectXTex_Desktop_2019_Win10"
				location (SourcesPath("Thirdparty/Render/DX12/DirectXTex/DirectXTex"))

				-- uuid, kind and language are mandatory
				uuid "371B9FA9-4C90-4AC6-A123-ACED756D6C77"
				kind "StaticLib"
				language "C++"
				
				-- inlude only DX12 platform
				configmap {
					["Win64_DX12"] = "x64",
					["Debug*"] = "Debug",
					["DebugOpt*"] = "Debug",
					["Profile*"] = "Release",
					["Release*"] = "Release"
				}
				
		PopGroup()
	PopGroup()
end
