#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"
#include <functional>

namespace Cyclone
{

class CDefaultApplication;
struct CDefaultApplicationParams;

using MainEntryCallback = std::function<void(int, char*[], void* PlatformDataRawPtr, UniquePtr<CDefaultApplication>& App, CDefaultApplicationParams& AppParams)>;

ENGINE_API int PlatformIndependentMain(int argc, char* argv[], void* PlatformDataRawPtr, MainEntryCallback EntryCallback);

} // namespace Cyclone
