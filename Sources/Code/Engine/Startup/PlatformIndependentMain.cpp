#include "PlatformIndependentMain.h"

#include "Engine/Framework/Impl/DefaultApplication.h"

namespace Cyclone
{

int PlatformIndependentMain(int argc, char* argv[])
{
    // #todo_fixme
    DefaultApplicationParams params{};
    //params.hInstance = hInstance;
    //params.lpCmdLine = lpCmdLine;
    //params.nCmdShow = nCmdShow;
    params.windowCaption = "Borealis: Editor";

    DefaultApplication app{};
    if (app.Init(params) == C_STATUS::C_STATUS_OK)
    {
        return app.Run();
    }


    return 0;
}

} // namespace Cyclone
