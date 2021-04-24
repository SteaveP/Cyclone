#include "PlatformIndependentMain.h"

#include "Engine/Framework/Impl/DefaultApplication.h"

namespace Cyclone
{

int PlatformIndependentMain(int argc, char* argv[], void* PlatformDataPtr, MainEntryCallback EntryCallback)
{
    if (EntryCallback)
    {
        EntryCallback(argc, argv, PlatformDataPtr);
    }

    DefaultApplicationParams params{};
    params.PlatformDataPtr = PlatformDataPtr;
    params.WindowCaption = "Borealis: Editor";

    DefaultApplication app{};

    if (app.Init(params) == C_STATUS::C_STATUS_OK)
    {
        return app.Run();
    }

    return -1;
}

} // namespace Cyclone
