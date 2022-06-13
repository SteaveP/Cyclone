#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"
#include <functional>

namespace Cyclone
{

class DefaultApplication;
struct DefaultApplicationParams;

using MainEntryCallback = std::function<void(int, char*[], void* PlatformDataPtr, Ptr<DefaultApplication>& App, DefaultApplicationParams& AppParams)>;

ENGINE_API int PlatformIndependentMain(int argc, char* argv[], void* PlatformDataPtr, MainEntryCallback EntryCallback);

} // namespace Cyclone
