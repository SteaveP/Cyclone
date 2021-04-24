include "Scripts/Common.lua"
include "Scripts/Engine.lua"
include "Scripts/Editor.lua"

local workspaceName = "Cyclone"
local workspaceExtension = ".sln"

workspace(workspaceName)
    location(AppPath(""))

    startproject "Editor"
    defaultplatform "Win64_Vulkan"
    -- defaultconfiguration "DebugUnityLIB"

    SetupDefaultWorkspaceState()

    --IncludeCycloneEngine()
    IncludeEngine()
    IncludeEditor()
    --IncludeTools()
    --IncludeBorealisEditor()

    -- #todo
    -- premake.override(premake.main, 'postAction', function(base)
    --     -- copy solution to application dir
    --     os.copyfile(AppPath("Intermediate/Build/") .. workspaceName .. workspaceExtension, AppPath(workspaceName .. workspaceExtension))
    -- end)
