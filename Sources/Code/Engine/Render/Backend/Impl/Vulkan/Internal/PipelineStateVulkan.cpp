#include "PipelineStateVulkan.h"

#include "Engine/Render/Mesh.h"
#include "Engine/Render/Backend/Resource.h"

#include "RenderBackendVulkan.h"
#include "CommandBufferVulkan.h"
#include "ResourceManagerVulkan.h"

namespace Cyclone::Render
{

CPipelineStateVulkan::CPipelineStateVulkan() = default;
CPipelineStateVulkan::CPipelineStateVulkan(CPipelineStateVulkan&& Other) noexcept : CPipelineState(MoveTemp(Other))
{
    std::swap(m_Pipeline, Other.m_Pipeline);
    std::swap(m_BindPoint, Other.m_BindPoint);
    std::swap(m_Layout, Other.m_Layout);
    std::swap(m_DescriptorSets, Other.m_DescriptorSets);
    std::swap(m_BackendVk, Other.m_BackendVk);
}
CPipelineStateVulkan& CPipelineStateVulkan::operator =(CPipelineStateVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CPipelineState::operator =(MoveTemp(Other));
        std::swap(m_Pipeline, Other.m_Pipeline);
        std::swap(m_BindPoint, Other.m_BindPoint);
        std::swap(m_Layout, Other.m_Layout);
        std::swap(m_DescriptorSets, Other.m_DescriptorSets);
        std::swap(m_BackendVk, Other.m_BackendVk);
    }
    return *this;
}
CPipelineStateVulkan::~CPipelineStateVulkan()
{
    DeInitImpl();
    CASSERT(m_Pipeline == VK_NULL_HANDLE);
}

C_STATUS CPipelineStateVulkan::Init(const CPipelineStateDesc& Desc)
{
    CASSERT(m_Pipeline == VK_NULL_HANDLE);

    C_STATUS Result = CPipelineState::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_BackendVk = GET_BACKEND_IMPL(Desc.Backend);

    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);
    IResourceManager* ResourceM = m_BackendVk->GetResourceManagerVk(m_Desc.DeviceHandle);

    // Layout // #todo_vk cache whole PipelineLayout
    {
        m_DescriptorSets.resize(Desc.ShaderBindingSet.SetLayouts.size());
        for (uint32 i = 0; i < (uint32)m_DescriptorSets.size(); ++i)
            m_DescriptorSets[i] = Device.ResourceManager->GetDescriptorSetLayout(Desc.ShaderBindingSet.SetLayouts[i]);

        VkPipelineLayoutCreateInfo LayoutInfo{};
        VkDescriptorSetLayout Layouts[15]{};
        LayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        LayoutInfo.setLayoutCount = (uint32)m_DescriptorSets.size();
        LayoutInfo.pSetLayouts = LayoutInfo.setLayoutCount > 0 ? Layouts : nullptr;

        for (uint32 i = 0; i < LayoutInfo.setLayoutCount; ++i)
        {
            Layouts[i] = m_DescriptorSets[i]->Get();
        }

        VkResult ResultVk = VK_CALL(Device, vkCreatePipelineLayout(Device.DeviceVk, &LayoutInfo, nullptr, &m_Layout));
#if ENABLE_DEBUG_RENDER_BACKEND
        if (Desc.Name.empty() == false)
            SetDebugNameVk("PipelineLayout" + Desc.Name, VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64)m_Layout, Device);
#endif
        C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);
    }

    CASSERT(Desc.Flags == EPipelineFlags::None);
    VkPipelineCreateFlagBits Flags{};

    auto FillShader = [&](CShader* Shader, VkPipelineShaderStageCreateInfo& Info, VkShaderStageFlagBits StageType)
    {
        Info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        Info.stage = StageType;
        Info.module = (VkShaderModule)Shader->GetBackendDataPtr();
        Info.pName = Shader->GetDesc().EntryPoint.c_str();
    };

    VkResult ResultVk = VK_SUCCESS;
    if (Desc.Type == PipelineType::Graphics)
    {
        CASSERT(Desc.ComputeShader.IsValid() == false);
        m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkGraphicsPipelineCreateInfo Info{};
        Info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        Info.flags = Flags;
        Info.layout = m_Layout;

        Array<VkPipelineShaderStageCreateInfo, 2> ShaderStages{};
        Info.stageCount = 0;
        Info.pStages = ShaderStages.data();

        if (Desc.VertexShader.IsValid())
            FillShader(ResourceM->GetShader(Desc.VertexShader), ShaderStages[Info.stageCount++], VK_SHADER_STAGE_VERTEX_BIT);
        if (Desc.PixelShader.IsValid())
            FillShader(ResourceM->GetShader(Desc.PixelShader), ShaderStages[Info.stageCount++], VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
        VkVertexInputBindingDescription VertexInputBindings[10];
        VkVertexInputAttributeDescription VertexInputAttributes[10];
        {
            for (uint32 i = 0; i < Desc.VertexLayout.Bindings.size(); ++i)
            {
                auto& SrcBinding = Desc.VertexLayout.Bindings[i];

                VertexInputBindings[i].binding = SrcBinding.Index;
                VertexInputBindings[i].inputRate = ConvertVertexBindingRate(SrcBinding.BindingRate);
                VertexInputBindings[i].stride = SrcBinding.Stride;
            }

            for (uint32 i = 0; i < Desc.VertexLayout.Attributes.size(); ++i)
            {
                auto& SrcAttribute = Desc.VertexLayout.Attributes[i];

                VertexInputAttributes[i].binding = SrcAttribute.BindingIndex;
                VertexInputAttributes[i].location = SrcAttribute.SlotIndex;
                VertexInputAttributes[i].format = ConvertFormatType(SrcAttribute.Format);
                VertexInputAttributes[i].offset = SrcAttribute.Offset;
            }

            VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            VertexInputInfo.vertexBindingDescriptionCount = (uint32)Desc.VertexLayout.Bindings.size();
            VertexInputInfo.pVertexBindingDescriptions = VertexInputBindings;
            VertexInputInfo.vertexAttributeDescriptionCount = (uint32)Desc.VertexLayout.Attributes.size();
            VertexInputInfo.pVertexAttributeDescriptions = VertexInputAttributes;
        }
        Info.pVertexInputState = &VertexInputInfo;

        VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo{};
        InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        InputAssemblyInfo.topology = ConvertVertexBindingRate(Desc.PrimitiveTopology);
        InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
        Info.pInputAssemblyState = &InputAssemblyInfo;

        // Viewport is dynamic state so don't need to be initialized at creation
        VkViewport Viewport{};
        VkRect2D Scissor{};

        VkPipelineViewportStateCreateInfo ViewportInfo{};
        ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        ViewportInfo.viewportCount = 1;
        ViewportInfo.pViewports = &Viewport;
        ViewportInfo.scissorCount = 1;
        ViewportInfo.pScissors = &Scissor;
        Info.pViewportState = &ViewportInfo;

        VkPipelineRasterizationStateCreateInfo Rasterizer{};
        const CRasterizerState* RastState = m_BackendVk->GetRenderer()->GetRasterizerState(Desc.Rasterizer);
        Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        Rasterizer.depthClampEnable = RastState->DepthClampEnable;
        Rasterizer.rasterizerDiscardEnable = VK_FALSE;
        Rasterizer.polygonMode = ConvertPolygonFillMode(RastState->PolygonFillMode);
        Rasterizer.lineWidth = RastState->LineWidth;
        Rasterizer.cullMode = ConvertCullMode(RastState->CullMode);
        Rasterizer.frontFace = RastState->IsfrontFaceCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
        Rasterizer.depthBiasEnable = RastState->DepthBiasEnable;
        Rasterizer.depthBiasClamp = RastState->DepthBiasClamp;
        Rasterizer.depthBiasConstantFactor = RastState->DepthBiasConstantFactor;
        Rasterizer.depthBiasSlopeFactor = RastState->DepthBiasSlopeFactor;
        Info.pRasterizationState = &Rasterizer;

#if 0
        VkPipelineMultisampleStateCreateInfo Multisampling{};
        Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        Multisampling.sampleShadingEnable = VK_FALSE;
        Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        Multisampling.minSampleShading = 1.f;
        Multisampling.pSampleMask = nullptr;
        Multisampling.alphaToCoverageEnable = VK_FALSE;
        Multisampling.alphaToOneEnable = VK_FALSE;
        Info.pMultisampleState = &Multisampling;
#endif

        VkPipelineDepthStencilStateCreateInfo DepthStencilInfo{};
        const CDepthStencilState* DepthState = m_BackendVk->GetRenderer()->GetDepthStencilState(Desc.DepthStencil);
        DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        DepthStencilInfo.depthTestEnable = DepthState->DepthTestEnable;
        DepthStencilInfo.depthWriteEnable = DepthState->DepthWriteEnable;
        DepthStencilInfo.depthCompareOp = ConvertOperationMode(DepthState->DepthCompareOp);
        DepthStencilInfo.depthBoundsTestEnable = DepthState->DepthBoundsTestEnable;
        DepthStencilInfo.minDepthBounds = DepthState->MinDepthBounds;
        DepthStencilInfo.maxDepthBounds = DepthState->MaxDepthBounds;
        DepthStencilInfo.stencilTestEnable = DepthState->StencilTestEnable;
        DepthStencilInfo.front = {}; // #todo_vk_stencil
        DepthStencilInfo.back = {};
        Info.pDepthStencilState = &DepthStencilInfo;

        const CBlendState* BlendState = m_BackendVk->GetRenderer()->GetBlendState(Desc.Blend);
        VkPipelineColorBlendAttachmentState ColorBlendAttachments[10];
        VkPipelineColorBlendStateCreateInfo ColorBlending{};
        ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        ColorBlending.logicOpEnable = VK_FALSE;
        ColorBlending.logicOp = VK_LOGIC_OP_COPY;
        ColorBlending.blendConstants[0] = 0.f;
        ColorBlending.blendConstants[1] = 0.f;
        ColorBlending.blendConstants[2] = 0.f;
        ColorBlending.blendConstants[3] = 0.f;
        ColorBlending.attachmentCount = (uint32)Desc.RenderTargets.size();
        ColorBlending.pAttachments = ColorBlendAttachments;

        for (uint32 i = 0; i < ColorBlending.attachmentCount; ++i)
        {
            auto& ColorBlendAttachment = ColorBlendAttachments[i];
            ColorBlendAttachment.colorWriteMask = BlendState->ColorWriteMask;
            ColorBlendAttachment.blendEnable = BlendState->BlendEnable;
            CASSERT(ColorBlendAttachment.blendEnable == false);
            ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        };
        Info.pColorBlendState = &ColorBlending;

        VkPipelineDynamicStateCreateInfo DynamicStateInfo{};
        DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        Array<VkDynamicState, 10> DynamicState{};
        DynamicState[0] = VK_DYNAMIC_STATE_VIEWPORT;
        DynamicState[1] = VK_DYNAMIC_STATE_SCISSOR;
        DynamicStateInfo.dynamicStateCount = 2;
        DynamicStateInfo.pDynamicStates = DynamicState.data();
        Info.pDynamicState = &DynamicStateInfo;

        Info.renderPass = nullptr;
        Info.subpass = 0;

        VkPipelineRenderingCreateInfo DynamicRendInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        VkFormat ColorAttachmentFormats[10]{};
        {
            DynamicRendInfo.colorAttachmentCount = (uint32)Desc.RenderTargets.size();
            for (uint32 i = 0; i < DynamicRendInfo.colorAttachmentCount; ++i)
            {
                ColorAttachmentFormats[i] = ConvertFormatType(Desc.RenderTargets[i]);
            }
            DynamicRendInfo.pColorAttachmentFormats = ColorAttachmentFormats;

            if (Desc.DepthTarget)
            {
                DynamicRendInfo.depthAttachmentFormat = ConvertFormatType(Desc.DepthTarget.value());
                DynamicRendInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED; // #todo_vk_Stencil
            }
            else
            {
                DynamicRendInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
                DynamicRendInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
            }
        }
        Info.pNext = &DynamicRendInfo;


        ResultVk = VK_CALL(Device, vkCreateGraphicsPipelines(Device.DeviceVk, nullptr, 1, &Info, nullptr, &m_Pipeline));

#if ENABLE_DEBUG_RENDER_BACKEND
        if (Desc.Name.empty() == false)
        {
            SetDebugNameVk(Desc.Name, VK_OBJECT_TYPE_PIPELINE, (uint64)m_Pipeline, Device);
        }
#endif
    }
    else if (Desc.Type == PipelineType::Compute)
    {
        CASSERT(Desc.PixelShader.IsValid() == false && Desc.VertexShader.IsValid() == false);
        m_BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

        VkComputePipelineCreateInfo ComputeInfo{};
        ComputeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        ComputeInfo.flags = Flags;
        ComputeInfo.layout = m_Layout;

        if (Desc.ComputeShader.IsValid())
            FillShader(ResourceM->GetShader(Desc.VertexShader), ComputeInfo.stage, VK_SHADER_STAGE_COMPUTE_BIT);

        ResultVk = VK_CALL(Device, vkCreateComputePipelines(Device.DeviceVk, nullptr, 1, &ComputeInfo, nullptr, &m_Pipeline));
    }
    else
    {
        CASSERT(false);
    }

    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);
    
    return C_STATUS::C_STATUS_OK;
}

void CPipelineStateVulkan::DeInit()
{
    DeInitImpl();
    CPipelineState::DeInit();
}

void CPipelineStateVulkan::DeInitImpl() noexcept
{
    if (m_Pipeline)
    {
        m_BackendVk->GetDisposalManagerVk(m_Desc.DeviceHandle)->AddDisposable([Backend = m_BackendVk, DeviceHandle = m_Desc.DeviceHandle, 
            Pipeline = m_Pipeline, Layout = m_Layout]()
        {
            const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);
            VK_CALL(Device, vkDestroyPipeline(Device.DeviceVk, Pipeline, nullptr));
            VK_CALL(Device, vkDestroyPipelineLayout(Device.DeviceVk, Layout, nullptr));
        });

        m_Pipeline = VK_NULL_HANDLE;
        m_Layout = VK_NULL_HANDLE;
    }

    if (m_BackendVk && m_DescriptorSets.empty() == false)
    {
        const auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);
        for (uint32 i = 0; i < (uint32)m_DescriptorSets.size(); ++i)
        {
            Device.ResourceManager->ReleaseDescriptorSetLayout(m_DescriptorSets[i]->GetDesc());
        }
        m_DescriptorSets.clear();
    }
}

C_STATUS CPipelineStateVulkan::Bind(CCommandBufferVulkan* CommandBufferVk)
{
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);
    VK_CALL(Device, vkCmdBindPipeline(CommandBufferVk->Get(), m_BindPoint, m_Pipeline));

    return C_STATUS::C_STATUS_OK;
}

CDescriptorSetLayoutVulkan::~CDescriptorSetLayoutVulkan()
{
    DeInit();
}

C_STATUS CDescriptorSetLayoutVulkan::Init(CRenderBackendVulkan* BackendVk, CDeviceHandle DeviceHandle, const CDescriptorSetLayout& Desc)
{
    m_BackendVk = BackendVk;
    m_DeviceHandle = DeviceHandle;
    m_Desc = Desc;

    Array<VkDescriptorSetLayoutBinding, 10> LayoutBindings{};
    Array<VkDescriptorBindingFlags, 10> Flags{};
    VkDescriptorSetLayoutBindingFlagsCreateInfo FlagsInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = (uint32)Desc.Bindings.size(),
        .pBindingFlags = Flags.data()
    };

    VkDescriptorSetLayoutCreateInfo LayoutInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    LayoutInfo.bindingCount = (uint32)Desc.Bindings.size();
    LayoutInfo.pBindings = LayoutBindings.data();
    LayoutInfo.pNext = &FlagsInfo;
    for (uint32 i = 0; i < LayoutInfo.bindingCount; ++i)
    {
        auto& Binding = LayoutBindings[i];
        auto& BindingDesc = Desc.Bindings[i];
        Binding.binding = BindingDesc.BindingIndex;
        Binding.descriptorType = ConvertDescriptorType(BindingDesc.DescriptorType);
        Binding.descriptorCount = BindingDesc.DescriptorCount;
        Binding.stageFlags = BindingDesc.StageFlags == 0xFFFFFFFF ? VK_SHADER_STAGE_ALL : 0;

        if (BindingDesc.DescriptorType == EDescriptorType::Sampler && BindingDesc.ImmutableSamplers.size() > 0)
        {
            CASSERT(BindingDesc.DescriptorCount == BindingDesc.ImmutableSamplers.size());

            CResourceManagerVulkan* ResourceM = m_BackendVk->GetResourceManagerVk(m_DeviceHandle);
            CASSERT(ResourceM);

            VkSampler Samplers[10]{};
            for (uint32 j = 0; j < BindingDesc.DescriptorCount; ++j)
            {
                CSamplerVulkan* SamplerPtr = BACKEND_DOWNCAST(ResourceM->GetSampler(ResourceM->GetSamplerCached(BindingDesc.ImmutableSamplers[j])), CSamplerVulkan);
                CASSERT(SamplerPtr);

                Samplers[j] = SamplerPtr->Get();
            }
            Binding.pImmutableSamplers = Samplers;
        }

        // #todo_vk VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT ?
        if (BindingDesc.Flags & EDescriptorSetLayoutFlags::Bindless)
        {
            // #todo_vk VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT 
            Flags[i] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
            LayoutInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }
    }

    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(DeviceHandle);
    VkResult Result = VK_CALL(Device, vkCreateDescriptorSetLayout(Device.DeviceVk, &LayoutInfo, nullptr, &m_DescriptorSetLayoutVk));
    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void CDescriptorSetLayoutVulkan::DeInit()
{
    if (m_DescriptorSetLayoutVk)
    {
        m_BackendVk->GetDisposalManagerVk(m_DeviceHandle)->AddDisposable([Backend = m_BackendVk, DeviceHandle = m_DeviceHandle,
            Layout = m_DescriptorSetLayoutVk]()
        {
            const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);
            VK_CALL(Device, vkDestroyDescriptorSetLayout(Device.DeviceVk, Layout, nullptr));
        });
        m_DescriptorSetLayoutVk = nullptr;

        CResourceManagerVulkan* ResourceManVk = m_BackendVk->GetResourceManagerVk(m_DeviceHandle);
        for (uint32 i = 0; i < (uint32)m_Desc.Bindings.size(); ++i)
        {
            auto& BindingDesc = m_Desc.Bindings[i];
            if (BindingDesc.DescriptorType == EDescriptorType::Sampler && BindingDesc.ImmutableSamplers.size() > 0)
            {
                CASSERT(BindingDesc.DescriptorCount == BindingDesc.ImmutableSamplers.size());

                for (uint32 j = 0; j < BindingDesc.DescriptorCount; ++j)
                {
                    ResourceManVk->ReleaseSamplerCached(BindingDesc.ImmutableSamplers[j]);
                }
            }
        }
    }
}

C_STATUS CDescriptorSetVk::Bind(CCommandBufferVulkan* CommandBufferVk)
{
    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_DeviceHandle);
 
    // #todo_vk
    //auto& Device = m_BackendVk->GetGlobalContext().GetLogicalDevice(CommandBufferVk->GetDevice());
    //VK_CALL(Device, vkCmdBindDescriptorSets(CommandBufferVk->Get(), m_BindPoint, m_Pipeline->GetLayout(), 0, 1, &m_descriptorSets[i], 0, nullptr);
 
    return C_STATUS::C_STATUS_OK;
}

CSamplerVulkan::CSamplerVulkan() = default;
CSamplerVulkan::CSamplerVulkan(CSamplerVulkan&& Other) noexcept : CSampler(MoveTemp(Other))
{
    std::swap(m_SamplerVk, Other.m_SamplerVk);
    std::swap(m_BackendVk, Other.m_BackendVk);
}
CSamplerVulkan& CSamplerVulkan::operator=(CSamplerVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CSampler::operator=(MoveTemp(Other));
        std::swap(m_SamplerVk, Other.m_SamplerVk);
        std::swap(m_BackendVk, Other.m_BackendVk);
    }
    return *this;
}
CSamplerVulkan::~CSamplerVulkan()
{
    DeInitImpl();
}

C_STATUS CSamplerVulkan::Init(const CSamplerDesc& Desc)
{
    C_STATUS Result = CSampler::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    m_BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);

    // #todo_vk_sampler
    VkSamplerCreateInfo Info{ .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    Info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    Info.magFilter = VK_FILTER_LINEAR;
    Info.minFilter = VK_FILTER_LINEAR;
    Info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    Info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    Info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    Info.anisotropyEnable = VK_FALSE;
    Info.maxAnisotropy = 16.0f;
    Info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    Info.unnormalizedCoordinates = VK_FALSE;
    Info.compareEnable = VK_FALSE;
    Info.compareOp = VK_COMPARE_OP_ALWAYS;

    Info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    Info.mipLodBias = 0.0f;
    Info.minLod = 0.0f;
    Info.maxLod = 16.f;

    auto& Device = m_BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

    VkResult ResultVk = VK_CALL(Device, vkCreateSampler(Device.DeviceVk, &Info, nullptr, &m_SamplerVk));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void CSamplerVulkan::DeInit()
{
    DeInitImpl();
    CSampler::DeInit();
}

void CSamplerVulkan::DeInitImpl() noexcept
{
    if (m_SamplerVk)
    {
        m_BackendVk->GetDisposalManagerVk(m_Desc.DeviceHandle)->AddDisposable([Backend = m_BackendVk, DeviceHandle = m_Desc.DeviceHandle,
            Sampler = m_SamplerVk]()
            {
                const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);
                VK_CALL(Device, vkDestroySampler(Device.DeviceVk, Sampler, nullptr));
            });

        m_SamplerVk = nullptr;
    }
}

} //namespace Cyclone::Render
