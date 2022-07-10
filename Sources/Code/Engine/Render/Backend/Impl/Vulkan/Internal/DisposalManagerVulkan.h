#pragma once

#include "CommonVulkan.h"

#include <functional>

namespace Cyclone::Render
{

class CDisposalManagerDesc
{
public:
};

class CDisposalManagerVulkan
{
public:
    using CDisposalFunc = std::function<void()>;

public:
    DISABLE_COPY_ENABLE_MOVE(CDisposalManagerVulkan);

    CDisposalManagerVulkan();
    virtual ~CDisposalManagerVulkan();

    C_STATUS Init(CRenderBackendVulkan* BackendVk, CDisposalManagerDesc Desc);
    void DeInit();

    void Tick(bool IsShutdowning = false);

    // This func will be called when frame at was added already done on GPU
    // Note that if you use labda, need to capture closure by value 
    // because at the calling time original object may be already destroyed
    void AddDisposable(CDisposalFunc Func);

protected:
    Vector<Pair<uint64, CDisposalFunc>> m_PendingDisposables;

    CRenderBackendVulkan* m_BackendVk = nullptr;
    CDisposalManagerDesc m_Desc;
};

} // namespace Cyclone::Render
