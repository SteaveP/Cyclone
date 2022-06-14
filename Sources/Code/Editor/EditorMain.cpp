#include "Engine/Engine.h"
#include "EditorApp.h"

static auto AppCreateCallback = [](int Argc, char* Argv[], void* PlatformDataPtr,
    Cyclone::Ptr<Cyclone::DefaultApplication>& App, Cyclone::DefaultApplicationParams& AppParams)
{
    App = std::make_shared<Cyclone::EditorApplication>();
    AppParams.WindowCaption = "Cyclone's Editor";
};

#define APPLICATION_CRATE_CALLBACK AppCreateCallback

#define GENERATE_MAIN_FUNCTION 1
#define GENERATE_DEFAULT_MODULE_LOADER 1
#include "Engine/Startup/DefaultMain.cpp"
