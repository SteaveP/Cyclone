#pragma once

#include "CommonVulkan.h"

#include "Engine/Render/Backend/Pipeline.h"

namespace Cyclone::Render
{

class CSamplerVulkan : public CSampler // #todo_vk_sampler
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CSamplerVulkan);
    CSamplerVulkan();
    virtual ~CSamplerVulkan();

    virtual C_STATUS Init(const CSamplerDesc& Desc) override;
    virtual void DeInit() override;

    VkSampler Get() const { return m_SamplerVk; }

private:
    void DeInitImpl() noexcept;

protected:
    VkSampler m_SamplerVk = VK_NULL_HANDLE;
    CRenderBackendVulkan* m_BackendVk = nullptr;
};

class CDescriptorSetLayoutVulkan
{
public:
    ~CDescriptorSetLayoutVulkan();

    C_STATUS Init(CRenderBackendVulkan* BackendVk, CDeviceHandle DeviceHandle, const CDescriptorSetLayout& Desc);
    void DeInit();

    VkDescriptorSetLayout Get() const { return m_DescriptorSetLayoutVk; }
    const CDescriptorSetLayout& GetDesc() const { return m_Desc; }

protected:
    VkDescriptorSetLayout m_DescriptorSetLayoutVk = VK_NULL_HANDLE;
    CRenderBackendVulkan* m_BackendVk = nullptr;
    CDeviceHandle m_DeviceHandle;
    CDescriptorSetLayout m_Desc;
};

struct CShaderBindingSetVulkan
{
    Vector<CDescriptorSetLayoutVulkan*> DescriptorSetLayouts;
};

class CPipelineStateVulkan : public CPipelineState
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CPipelineStateVulkan);

    CPipelineStateVulkan();
    virtual ~CPipelineStateVulkan();

    virtual C_STATUS Init(const CPipelineStateDesc& Desc) override;
    virtual void DeInit() override;

    virtual void* GetBackendDataPtr() const override { return m_Pipeline; }

    C_STATUS Bind(CCommandBufferVulkan* CommandBufferVk);

    VkPipeline Get() const { return m_Pipeline; }
    VkPipelineLayout GetLayout() const { return m_Layout; }
    VkPipelineBindPoint GetBindPoint() const { return m_BindPoint; }

private:
    void DeInitImpl() noexcept;

protected:
    VkPipeline m_Pipeline = VK_NULL_HANDLE;
    VkPipelineBindPoint m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VkPipelineLayout m_Layout = VK_NULL_HANDLE;

    Vector<CDescriptorSetLayoutVulkan*> m_DescriptorSets; // #todo_vk optimize
    CRenderBackendVulkan* m_BackendVk = nullptr;
};


class CDescriptorSetVk // #todo_vk_material use this class
{
public:
    ~CDescriptorSetVk();

    C_STATUS Init(CRenderBackendVulkan* BackendVk, CDeviceHandle DeviceHandle, const CDescriptorSetLayout& Desc);
    void DeInit();

    C_STATUS Bind(CCommandBufferVulkan* CommandBufferVk);

    VkDescriptorSet Get() const { return m_DescriptorSetVk; }
    //VkDescriptorSetLayout GetLayout() const { return m_DescriptorSetLayoutVk; }
    //const CDescriptorSetLayout& GetDesc() const { return m_Desc; }

protected:
    VkDescriptorSet m_DescriptorSetVk = VK_NULL_HANDLE;
    //VkDescriptorSetLayout m_DescriptorSetLayoutVk = VK_NULL_HANDLE; // cache
    CPipelineStateVulkan* m_Pipeline = nullptr;
    VkPipelineBindPoint m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    CDescriptorSetLayoutVulkan* m_Layout = nullptr;
    CRenderBackendVulkan* m_BackendVk = nullptr;
    CDeviceHandle m_DeviceHandle;
};

} // namespace Cyclone::Render
