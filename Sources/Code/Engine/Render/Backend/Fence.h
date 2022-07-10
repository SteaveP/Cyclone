#pragma once

#include "Engine/Render/CoreRender.h"

namespace Cyclone::Render
{

class ENGINE_API CFenceDesc
{
public:
    uint64 InitialValue = 0;
    IRendererBackend* Backend = nullptr;

    CDeviceHandle DeviceHandle;

#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif
};

// DX12-Style Fence with monotonically increasing value
class ENGINE_API CFence
{
public:
    DISABLE_COPY_ENABLE_MOVE(CFence);

    CFence();
    virtual ~CFence();

    virtual C_STATUS Init(const CFenceDesc& Desc);
    virtual void DeInit() {}

    virtual C_STATUS SignalFromDevice(CCommandQueue* CommandQueue, uint64 Value) = 0;
    virtual C_STATUS SignalFromHost(uint64 Value) = 0;

    virtual C_STATUS WaitFromHost(uint64 Value, uint64 Timeout = UINT64_MAX) = 0;

    virtual uint64 GetValue() const { return m_PendingValue; }
    virtual uint64 GetCompletedValue() const = 0;

    virtual void IncrementAndSignalFromHost() = 0;
    virtual void IncrementAndSignalFromDevice(CCommandQueue* CommandQueue) = 0;

    const CFenceDesc& GetDesc() const { return m_Desc; }

public:
    uint64 m_PendingValue = 0; // #todo_vk #todo_mt should be atomic
    CFenceDesc m_Desc;
};

} // namespace Cyclone::Render
