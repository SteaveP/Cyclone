#include "EventWin.h"

#include "Window/WindowWin.h"

namespace Cyclone
{

CEventWin::CEventWin() = default;
CEventWin::CEventWin(CEventWin&& Other) noexcept : IEvent(MoveTemp(Other))
{
    std::swap(m_Event, Other.m_Event);
    std::swap(m_Name, Other.m_Name);
}
CEventWin& CEventWin::operator=(CEventWin&& Other) noexcept
{
    if (this != &Other)
    {
        IEvent::operator=(MoveTemp(Other));
        std::swap(m_Event, Other.m_Event);
        std::swap(m_Name, Other.m_Name);
    }
    return *this;
}
CEventWin::~CEventWin()
{
    DeInit();
}

Cyclone::C_STATUS CEventWin::Init(String Name, EEventFlags Flags)
{
    bool ManualReset = (Flags & EEventFlags::AutoReset) == 0;
    bool FlaggedState = (Flags & EEventFlags::FlaggedState);
    m_Event = CreateEventA(nullptr, ManualReset, FlaggedState, Name.c_str());
    m_Name = MoveTemp(Name);

    return C_STATUS::C_STATUS_OK;
}

void CEventWin::DeInit()
{
    if (m_Event)
    {
        CloseHandle(m_Event);
        m_Event = INVALID_HANDLE_VALUE;
    }
}

Cyclone::C_STATUS CEventWin::Wait(uint32 TimeoutInMilliseconds)
{
    BOOL Allertable = FALSE;
    DWORD Result = WaitForSingleObjectEx(m_Event, TimeoutInMilliseconds, Allertable);
    C_ASSERT_RETURN_VAL(SUCCEEDED(Result), C_STATUS::C_STATUS_ERROR);
    // #todo_vk check timeout state
    return C_STATUS::C_STATUS_OK;
}

void CEventWin::Signal()
{
    DWORD Result = SetEvent(m_Event);
    CASSERT(SUCCEEDED(Result));
}

void CEventWin::Reset()
{
    DWORD Result = ResetEvent(m_Event);
    CASSERT(SUCCEEDED(Result));
}

} // namespace Cyclone
