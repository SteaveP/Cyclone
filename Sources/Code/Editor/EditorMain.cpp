#include "Engine/Engine.h"
#include "EditorApp.h"

static auto AppCreateCallback = [](int Argc, char* Argv[], void* PlatformDataPtr,
    Cyclone::UniquePtr<Cyclone::CDefaultApplication>& App, Cyclone::CDefaultApplicationParams& AppParams)
{
    App = Cyclone::MakeUnique<Cyclone::CEditorApplication>();
    AppParams.WindowCaption = "Cyclone's Editor";
};

#define APPLICATION_CREATE_CALLBACK AppCreateCallback

#define GENERATE_MAIN_FUNCTION 1
#define GENERATE_DEFAULT_MODULE_LOADER 1
#include "Engine/Startup/DefaultMain.cpp"
