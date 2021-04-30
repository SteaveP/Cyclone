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
    if (EntryCallback)
    {
        EntryCallback(argc, argv, PlatformDataPtr);
    }

    std::shared_ptr<Render::Renderer> DefaultRenderer = std::make_shared<Render::Renderer>();
    DefaultRenderer->PreInit(GEngineGetCurrentRenderBackend());

    DefaultApplicationParams params{};
    params.Platform = GEngineGetCurrentPlatform();
    params.Renderer = DefaultRenderer;
    params.InputManager = std::make_shared<DefaultInputManager>();
    params.UIModule = GEngineGetCurrentUIModule();
    params.PlatformStartupDataPtr = PlatformDataPtr;
#ifdef WINDOW_CAPTION
    params.WindowCaption = WINDOW_CAPTION;
#else
    params.WindowCaption = "Cyclone's Window";
#endif

    DefaultApplication app{};

    if (app.Init(params) == C_STATUS::C_STATUS_OK)
    {
        return app.Run();
    }

    return -1;
}

} // namespace Cyclone
