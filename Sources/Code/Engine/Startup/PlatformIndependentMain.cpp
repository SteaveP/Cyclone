#include "PlatformIndependentMain.h"

#include "Engine/Framework/IUISubsystem.h"
#include "Engine/Framework/Impl/DefaultApplication.h"
#include "Engine/Framework/Impl/DefaultInputManager.h"
#include "Engine/Render/IRendererBackend.h"
#include "Engine/Render/Renderer.h"

#include "Engine/Framework/IPlatform.h"

namespace Cyclone
{

int PlatformIndependentMain(int argc, char* argv[], void* PlatformDataPtr, MainEntryCallback EntryCallback)
{
    Ptr<DefaultApplication> App{};
    DefaultApplicationParams Params{};

    if (EntryCallback)
    {
        EntryCallback(argc, argv, PlatformDataPtr, App, Params);
    }

    Ptr<Render::Renderer> DefaultRenderer = MakeShared<Render::Renderer>();
    DefaultRenderer->PreInit(GEngineGetCurrentRenderBackend());

    Params.Platform = GEngineGetCurrentPlatform();
    Params.Renderer = MoveTemp(DefaultRenderer);
    Params.InputManager = MakeShared<DefaultInputManager>();
    Params.UIModule = GEngineGetCurrentUISubsystem();
    Params.PlatformStartupDataPtr = PlatformDataPtr;

    if (Params.WindowCaption.empty())
        Params.WindowCaption = "Cyclone's Window";

    if (App == nullptr)
        App = MakeShared<DefaultApplication>();

    if (App->Init(Params) == C_STATUS::C_STATUS_OK)
    {
        return App->Run();
    }

    return -1;
}

} // namespace Cyclone
