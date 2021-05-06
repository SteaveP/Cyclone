#include "PlatformIndependentMain.h"

#include "Engine/Framework/Impl/DefaultApplication.h"
#include "Engine/Framework/Impl/DefaultInputManager.h"
#include "Engine/Render/Renderer.h"
#include "Engine/UI/ImGui/ImGuiModule.h"

#include "Engine/Framework/IPlatform.h"

namespace Cyclone
{

int PlatformIndependentMain(int argc, char* argv[], void* PlatformDataPtr, MainEntryCallback EntryCallback)
{
    std::shared_ptr<DefaultApplication> App;
    DefaultApplicationParams Params{};

    if (EntryCallback)
    {
        EntryCallback(argc, argv, PlatformDataPtr, App, Params);
    }

    std::shared_ptr<Render::Renderer> DefaultRenderer = std::make_shared<Render::Renderer>();
    DefaultRenderer->PreInit(GEngineGetCurrentRenderBackend());

    Params.Platform = GEngineGetCurrentPlatform();
    Params.Renderer = DefaultRenderer;
    Params.InputManager = std::make_shared<DefaultInputManager>();
    Params.UIModule = GEngineGetCurrentUIModule();
    Params.PlatformStartupDataPtr = PlatformDataPtr;

    if (Params.WindowCaption.empty())
        Params.WindowCaption = "Cyclone's Window";

    if (App == nullptr)
        App = std::make_shared<DefaultApplication>();

    if (App->Init(Params) == C_STATUS::C_STATUS_OK)
    {
        return App->Run();
    }

    return -1;
}

} // namespace Cyclone
