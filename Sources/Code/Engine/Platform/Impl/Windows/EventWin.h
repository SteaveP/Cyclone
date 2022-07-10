#pragma once

#include "PlatformWinModuleDefines.h"
#include "Engine/Platform/IEvent.h"

#include "Common/CommonWin.h"

namespace Cyclone
{

class CEventWin : public IEvent
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CEventWin);

    CEventWin();
    ~CEventWin();

    virtual C_STATUS Init(String Name, EEventFlags Flags);
    void DeInit();

    virtual C_STATUS Wait(uint32 TimeoutInMilliseconds= InfiniteTimeout) override;
    virtual void Signal() override;
    virtual void Reset() override;

    virtual const String& GetName() const override { return m_Name; }

protected:
    HANDLE m_Event = INVALID_HANDLE_VALUE;
    String m_Name;
};

} // namespace Cyclone
