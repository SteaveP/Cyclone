include "Scripts/Engine/Engine.lua"
include "Scripts/Editor/Editor.lua"

local workspaceName = "Cyclone"
local workspaceExtension = ".sln"

workspace(workspaceName)
    location(AppPath(""))

    -- temp workaround 
    os.mkdir(AppPath("Bin"))

    startproject "Editor"
    defaultplatform "Win64_Vulkan"

    SetupDefaultWorkspaceState()

    IncludeEngine()
    IncludeEditor()