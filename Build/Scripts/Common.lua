--------------------------------------------------------------------
-- Folder structure management
--------------------------------------------------------------------
ApplicationDir = path.join(_SCRIPT_DIR, "../..")
ApplicationIntermediateDir = path.join(ApplicationDir, "Intermediate/Build")
ApplicationSourceDir = path.join(ApplicationDir, "Sources")

-- make path relative to specified Dir
function AppPath(dir)
    return path.join(ApplicationDir, dir)
end
function SourcesPath(dir)
    return path.join(ApplicationSourceDir, dir)
end

function EnginePath(dir)
	return SourcesPath(path.join("Code/Engine", dir))
end

function BuildPath(dir)
	return AppPath(path.join("Build",dir))
end

--------------------------------------------------------------------
-- group management
--------------------------------------------------------------------
local currentGroup = ""

function SetGroup(name)
    currentGroup = name
    group(currentGroup)
end

function PushGroup(name)
    currentGroup = ((currentGroup ~= "") and (currentGroup .. "/") or  "") .. name
    --print("Push " .. currentGroup)
    group(currentGroup)
end

function PopGroup()
    currentGroup = path.getdirectory(currentGroup)

    if currentGroup == "." then
        currentGroup = ""
    end
    
    --print("Pop " .. currentGroup)
    group(currentGroup)
end

--------------------------------------------------------------------
-- Defaults
--------------------------------------------------------------------
function SetupDefaultWorkspaceState()
    configurations {
        -- static linking
        "DebugLIB", "DebugOptLIB", "ProfileLIB", "ReleaseLIB",
        "DebugUnityLIB", "DebugOptUnityLIB", "ProfileUnityLIB", "ReleaseUnityLIB",

        -- dynamic linking
        "DebugDLL", "DebugOptDLL", "ProfileDLL", "ReleaseDLL",
        "DebugUnityDLL", "DebugOptUnityDLL", "ProfileUnityDLL", "ReleaseUnityDLL",
    }

    platforms { "Win64_DX12", "Win64_Vulkan" }

    filter { "platforms:Win64*" }
        system "windows"
        architecture "x86_64"

        defines { "PLATFORM_WIN64"}

    filter { "configurations:Debug*"}
        symbols "On"
        defines { "_DEBUG", "DEBUG" }
        runtime "Debug"
        
    filter { "configurations:Profile* or Release*"}
        defines { "_NDEBUG", "NDEBUG" }
        runtime "Release"
        
        optimize "Full"
        
        flags { 
            "NoBufferSecurityCheck", "NoRuntimeChecks",
            "NoIncrementalLink", "LinkTimeOptimization",
        }

    filter { "configurations:DebugOpt*"}
        optimize "On"

    filter { "configurations:not Release*"}
        defines { 
            "PROFILE_ENABLED",
            "DEVELOPER",
            "DEVELOPER_TOOLS",
            "USE_RENDERDOC",
        }
    
    filter "kind:SharedLib or StaticLib"
        defines { "LIBRARY_TARGET", "LIBRARY_TARGET_%{prj.name:upper()}" }
        
    filter "kind:SharedLib"
        defines { "DYNAMIC_LIB", "DYNAMIC_LIB_%{prj.name:upper()}" }

    filter "kind:StaticLib"
        defines { "STATIC_LIB", "STATIC_LIB_%{prj.name:upper()}" }

    filter { "Debug*" }
        targetsuffix "_D"
    filter { "Profile*" }
        targetsuffix "_P"

    filter { "platforms:*DX12"}
        defines { "RENDER_BACKEND_DX12" }
        
    filter { "platforms:*Vulkan"}
        defines { "RENDER_BACKEND_VULKAN" }
    
	-- filter "system:windows"
    --     links       { "ole32", "ws2_32", "advapi32", "version" }

    filter "action:vs*"
        defines     { "_CRT_SECURE_NO_DEPRECATE", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_WARNINGS" }

    filter { } -- Reset the filter for other settings
    
    -- #todo for debug use cdecl?
    callingconvention "FastCall"
    
    flags { "MultiProcessorCompile" }
    cppdialect "c++17"
    staticruntime "Off"

    -- copy pdb's and exe to output dir!

    -- exceptionhandling
        -- "Default",
        -- "On",
        -- "Off",
        -- "SEH",
        -- "CThrow",

    -- editandcontinue
        -- "Default",
        -- "On",
        -- "Off",

    -- dpiawareness
        -- "Default",
        -- "None",
        -- "High",
        -- "HighPerMonitor",

    -- functionlevellinking 
        -- kind = "boolean"

    -- inlining
    -- 			"Default",
			-- "Disabled",
			-- "Explicit",
			-- "Auto"

    -- icon
        -- kind = "file"

    -- nativewchar
--    "Default",
--			"On",
--			"Off",

    -- name = "nuget",
	--	scope = "config",
	--	kind = "list:string",

   -- "rtti",
--			"Default",
--			"On",
--			"Off",

-- vectorextensions
    -- "Default",
    -- "AVX",
    -- "AVX2",
    -- "IA32",
    -- "SSE",
    -- "SSE2",
    -- "SSE3",
    -- "SSSE3",
    -- "SSE4.1",

    -- bool editorintegration - what is it?

    -- omitframepointer
    --			"Default",
    --			"On",
    --			"Off",

    -- for debug: 
	--		debugargs   { "--scripts=%{prj.location}/%{path.getrelative(prj.location, prj.basedir)}", "test" }
    --		debugdir    "."
    debugdir(AppPath("Bin"))

    includedirs ({ApplicationDir,
        AppPath("Sources/"),
        AppPath("Sources/Code/"),
    })

    --
    -- A more thorough cleanup.
    --
	if _ACTION == "clean" then
		--os.rmdir("bin")
		--os.rmdir("build")
	end
end

function SetupDefaultProjectState(projectName, projectKind)
    local intermediateFolder = path.join(ApplicationIntermediateDir, projectName)

    location(intermediateFolder)

    if (projectKind == "Library" ) then
        filter { "configurations:*LIB" }
            kind "StaticLib"
        filter { "configurations:*DLL" }
            kind "SharedLib"
        filter { }
    elseif (projectKind ~= "External") then
        kind(projectKind)
    end

	filter { "configurations:*Unity*" }
        UseUnityBuild "On" -- custom feature
    filter {}

    -- targetdir (path.join(intermediateFolder, "Bin/%{prj.name}/%{cfg.platform}/%{cfg.shortname}"))
    -- objdir (path.join(intermediateFolder, "Obj/%{prj.name}/%{cfg.platform}/%{cfg.shortname}"))
    -- targetdir (path.join(intermediateFolder, "Bin/%{cfg.shortname}"))
    -- objdir (path.join(intermediateFolder, "Obj/%{cfg.shortname}"))

    -- copy .exe or .dll file to <AppDir>/Bin folder after build
    -- #todo also copy PDB's
    if (projectKind ~= "External") then
        filter {"kind:WindowedApp or Sharedlib"}
            -- #todo add suffix for platform
            postbuildcommands { "{COPY} %{cfg.buildtarget.abspath} " .. AppPath("Bin/") }
        filter {}
    end
end

-----------------------------------------------------------------
-- Extension: adds ability to setup UnityBuilds for Visual Studio --
-----------------------------------------------------------------
require('vstudio')

local nname = "UseUnityBuild"
local field = premake.field.get(nname)
if field then
    premake.api.unregister(nname)
end
premake.api.register {
    name = nname,
    scope = "config",
    kind = "boolean"
}

local t = premake.vstudio.vc2010.elements
local function FeatureVStudioUnityBuildEnable(cfg)
    if _ACTION >= "vs2017" then
        if cfg.UseUnityBuild then
            -- "EnableUnitySupport" is actual XML element in project file
            premake.vstudio.vc2010.element("EnableUnitySupport", nil, "true")
        end
    end
end

function EnableUnityBuildFeature()
    premake.override(premake.vstudio.vc2010.elements, "configurationProperties", function(base,cfg)
        local calls = base(cfg)
        table.insert(calls, FeatureVStudioUnityBuildEnable)
        return calls
    end)
end

EnableUnityBuildFeature()
------------------------------------------------------------------