#pragma once

#include "Engine/Render/Backend/Fence.h"
#include "CommonVulkan.h"

namespace Cyclone::Render
{
   
class CFenceVulkan : public CFence
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CFenceVulkan);

    CFenceVulkan();
    virtual ~CFenceVulkan();

    virtual C_STATUS Init(const CFenceDesc& Desc) override;
    virtual void DeInit() override;

    virtual C_STATUS SignalFromDevice(CCommandQueue* CommandQueue, uint64 Value) override;
    virtual C_STATUS SignalFromHost(uint64 Value) override;

    virtual C_STATUS WaitFromHost(uint64 Value, uint64 Timeout = UINT64_MAX) override;

    virtual uint64 GetCompletedValue() const override;

    virtual void IncrementAndSignalFromHost() override;
    virtual void IncrementAndSignalFromDevice(CCommandQueue* CommandQueue) override;

    virtual VkSemaphore GetSemaphoreVk() const { return m_Semaphore; }

private:
    void DeInitImpl();

protected:
    VkSemaphore m_Semaphore = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
