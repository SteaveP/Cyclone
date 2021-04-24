#pragma once

#include "Engine/EngineModule.h"
#include <functional>

namespace Cyclone
{

using MainEntryCallback = std::function<void(int, char*[], void* PlatformDataPtr)>;

ENGINE_API int PlatformIndependentMain(int argc, char* argv[], void* PlatformDataPtr, MainEntryCallback EntryCallback);

} // namespace Cyclone
