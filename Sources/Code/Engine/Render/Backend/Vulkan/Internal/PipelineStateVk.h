#pragma once

#include "CommonVulkan.h"

#include "Engine/Render/Types/Pipeline.h"

namespace Cyclone::Render
{

class RenderBackendVulkan;
class RenderPassVk;
class DescriptorSetLayoutVk;

class DescriptorSetLayoutVk
{
public:
    VkDescriptorSetLayout LayoutVk = VK_NULL_HANDLE;
};

struct PipelineStateVkInitInfo
{
    const CPipelineDesc* PipelineDesc = nullptr;
    CDeviceHandle DeviceHandle;

    RenderPassVk* RenderPass = nullptr;
    DescriptorSetLayoutVk* DescriptorSetLayout = nullptr;
    RenderBackendVulkan* Backend = nullptr;
};

class PipelineStateVk
{
public:
    ~PipelineStateVk();

    C_STATUS Init(const PipelineStateVkInitInfo& InitInfo);
    void DeInit();

    C_STATUS Bind(CommandBufferVulkan* CommandBufferVk);

    VkPipeline Get() const { return m_Pipeline; }
    RenderPassVk* GetRenderPass() const { return m_RenderPass; }

protected:
    RenderPassVk* m_RenderPass = nullptr;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;
    VkPipelineBindPoint m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // #todo_vk_first
    VkPipelineLayout m_Layout = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
