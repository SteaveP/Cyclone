#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Core/Types.h"

namespace Cyclone
{

class IApplication;

class ENGINE_API ISubsystem
{
public:
    DISABLE_COPY_ENABLE_MOVE(ISubsystem);

    ISubsystem(const String& Name) : m_Name(Name) {};
    virtual ~ISubsystem() = default;

    virtual C_STATUS Init(IApplication* App) = 0;
    virtual void DeInit() = 0;

    virtual C_STATUS OnRegister() = 0;
    virtual void OnUnRegister() = 0;

    // #todo_subsystem can render, can update, etc

    const String& GetName() const { return m_Name; }

protected:
    String m_Name;
};

} // namespace Cyclone
