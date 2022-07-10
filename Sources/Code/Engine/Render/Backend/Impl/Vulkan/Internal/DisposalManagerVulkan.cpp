#include "DisposalManagerVulkan.h"

#include "RenderBackendVulkan.h"

namespace Cyclone::Render
{

CDisposalManagerVulkan::CDisposalManagerVulkan() = default;
CDisposalManagerVulkan::~CDisposalManagerVulkan()
{
    DeInit();

    CASSERT(m_PendingDisposables.empty());
}

void CDisposalManagerVulkan::AddDisposable(CDisposalFunc Func)
{
    m_PendingDisposables.emplace_back(m_BackendVk->GetRenderer()->GetCurrentFrame(), MoveTemp(Func));
}

C_STATUS CDisposalManagerVulkan::Init(CRenderBackendVulkan* BackendVk, CDisposalManagerDesc Desc)
{
    m_BackendVk = BackendVk;
    m_Desc = MoveTemp(Desc);
    return C_STATUS::C_STATUS_OK;
}

void CDisposalManagerVulkan::DeInit()
{
    if (m_BackendVk)
    {
        m_BackendVk->WaitGPU(); // #todo_vk is this required?

        Tick(true);

        m_BackendVk = nullptr;
    }
}

void CDisposalManagerVulkan::Tick(bool IsShutdowning)
{
    if (IsShutdowning)
    {
        for (auto it = m_PendingDisposables.begin(); it != m_PendingDisposables.end(); ++it)
        {
            it->second();
        }
        m_PendingDisposables.clear();
    }
    else
    {
        uint64 LastCompletedFrame = m_BackendVk->GetRenderer()->GetLastCompletedFrame();

        // #todo_vk keep vector sorted so we can optimize deletion
        for (auto it = m_PendingDisposables.begin(); it != m_PendingDisposables.end(); )
        {
            if (LastCompletedFrame >= it->first)
            {
                it->second();
                it = m_PendingDisposables.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

} //namespace Cyclone::Render
