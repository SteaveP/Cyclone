#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

enum class ENGINE_API EEventFlags
{
    None            = 0,
    FlaggedState    = bit(0),
    AutoReset       = bit(1),

    Count = 2
};
IMPEMENT_ENUM_BITFIELD(EEventFlags);

constexpr uint32 InfiniteTimeout = uint32(-1);

class ENGINE_API IEvent
{
public:
    DISABLE_COPY_ENABLE_MOVE(IEvent);

    IEvent() = default;
    virtual ~IEvent() = default;

    virtual C_STATUS Init(String Name, EEventFlags Flags) = 0;

    virtual C_STATUS Wait(uint32 TimeoutInMilliseconds= InfiniteTimeout) = 0;
    virtual void Signal() = 0;
    virtual void Reset() = 0;

    virtual const String& GetName() const = 0;
};

} // namespace Cyclone
