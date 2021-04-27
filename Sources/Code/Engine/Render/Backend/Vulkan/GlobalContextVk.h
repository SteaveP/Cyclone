#pragma once

#include "Common/CommonVulkan.h"

namespace Cyclone::Render
{

class RenderBackendVulkan;

class GlobalContextVk
{
public:
    C_STATUS Init(RenderBackendVulkan* RenderBackend);
    C_STATUS Shutdown();

    VkInstance GetInstance() const { return m_instance; }
    RenderBackendVulkan* GetBackend() const { return m_renderBackend; }

protected:
    C_STATUS createInstance();
    void destroyInstance();

    bool checkValidationlayerSupport();

protected:
    RenderBackendVulkan* m_renderBackend = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
