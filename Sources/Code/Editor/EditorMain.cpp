#include "Engine/Engine.h"
#include "EditorApp.h"

#define APPLICATION_CRATE_CALLBACK [](int argc, char* argv[], void* PlatformDataPtr, \
            std::shared_ptr<Cyclone::DefaultApplication>& App, Cyclone::DefaultApplicationParams& AppParams) \
{ \
    App = std::make_shared<Cyclone::EditorApplication>(); \
    AppParams.WindowCaption = "Cyclone's Editor"; \
}

#define GENERATE_MAIN_FUNCTION 1
#define GENERATE_DEFAULT_MODULE_LOADER 1
#include "Engine/Startup/DefaultMain.cpp"
