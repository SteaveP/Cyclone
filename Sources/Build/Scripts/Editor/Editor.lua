include "../Engine/Platform.lua"
include "../Engine/Render/Render.lua"

function IncludeEditor()
    
	local function EditorPath(dir)
		return SourcesPath("Code/Editor/" .. dir)
	end

    project "Editor"
	    SetupDefaultProjectState("Editor", "WindowedApp")

        AddEngineDependency()

        files {EditorPath("**.h"), EditorPath("**.cpp")}

		icon (SourcesPath("Assets/Textures/Icons/EditorIcon.ico"))

		vpaths {
			["Editor/*"] = { EditorPath("**.h"), EditorPath("**.cpp") }
		}

		filter { 'platforms:Win*' }
			files { BuildPath('Scripts/Editor/Editor.rc'), SourcesPath('Assets/Textures/Icons/EditorIcon.ico') }
			vpaths { ['Resources/*'] = { '**.rc', SourcesPath('**.ico') } }
		filter {}
end
