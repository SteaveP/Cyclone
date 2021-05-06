#pragma once

#include "Engine/EngineModule.h"
#include <functional>

namespace Cyclone
{

class DefaultApplication;
struct DefaultApplicationParams;

using MainEntryCallback = std::function<void(int, char*[], void* PlatformDataPtr, std::shared_ptr<DefaultApplication>& App, DefaultApplicationParams& AppParams)>;

ENGINE_API int PlatformIndependentMain(int argc, char* argv[], void* PlatformDataPtr, MainEntryCallback EntryCallback);

} // namespace Cyclone
