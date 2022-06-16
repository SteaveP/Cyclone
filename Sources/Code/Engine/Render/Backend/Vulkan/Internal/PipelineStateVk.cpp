#include "PipelineStateVk.h"

#include "Engine/Render/Types/Mesh.h"

#include "RenderBackendVulkan.h"
#include "CommandBufferVulkan.h"
#include "RenderPassVk.h"

namespace Cyclone::Render
{

PipelineStateVk::~PipelineStateVk()
{
    DeInit();
    CASSERT(m_Pipeline == VK_NULL_HANDLE);
}

C_STATUS PipelineStateVk::Init(const PipelineStateVkInitInfo& InitInfo)
{
    CASSERT(m_Pipeline == VK_NULL_HANDLE);

    m_RenderPass = InitInfo.RenderPass;

    VkDevice Device = InitInfo.Backend->GetGlobalContext().GetLogicalDevice(m_RenderPass->GetDevice()).LogicalDeviceHandle;

    // Layout
    {
        VkPipelineLayoutCreateInfo LayoutInfo{};
        LayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        if (InitInfo.DescriptorSetLayout && InitInfo.DescriptorSetLayout->LayoutVk)
        {
            LayoutInfo.setLayoutCount = 1;
            LayoutInfo.pSetLayouts = &InitInfo.DescriptorSetLayout->LayoutVk;
        }

        VkResult Result = vkCreatePipelineLayout(Device, &LayoutInfo, nullptr, &m_Layout);
        C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
    }

    CASSERT(InitInfo.PipelineDesc->Flags == EPipelineFlags::None);
    VkPipelineCreateFlagBits Flags{};

    auto FillShader = [&](CShader* Shader, VkPipelineShaderStageCreateInfo& Info, VkShaderStageFlagBits StageType)
    {
        Info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        Info.stage = StageType;
        Info.module = (VkShaderModule)Shader->GetBackendDataPtr();
        Info.pName = Shader->GetDesc().EntryPoint.c_str();
    };

    VkResult Result = VK_SUCCESS;
    if (InitInfo.PipelineDesc->Type == PipelineType::Graphics)
    {
        CASSERT(InitInfo.PipelineDesc->ComputeShader == nullptr);
        m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkGraphicsPipelineCreateInfo Info{};
        Info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        Info.flags = Flags;
        Info.layout = m_Layout;

        Array<VkPipelineShaderStageCreateInfo, 2> ShaderStages{};
        Info.stageCount = 0;
        Info.pStages = ShaderStages.data();

        if (InitInfo.PipelineDesc->VertexShader)
            FillShader(InitInfo.PipelineDesc->VertexShader.get(), ShaderStages[Info.stageCount++], VK_SHADER_STAGE_VERTEX_BIT);
        if (InitInfo.PipelineDesc->PixelShader)
            FillShader(InitInfo.PipelineDesc->PixelShader.get(), ShaderStages[Info.stageCount++], VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
        VkVertexInputBindingDescription VertexInputBindings[10];
        VkVertexInputAttributeDescription VertexInputAttributes[10];
        {
            for (uint32 i = 0; i < InitInfo.PipelineDesc->VertexLayout.Bindings.size(); ++i)
            {
                auto& SrcBinding = InitInfo.PipelineDesc->VertexLayout.Bindings[i];

                VertexInputBindings[i].binding = SrcBinding.Index;
                VertexInputBindings[i].inputRate = ConvertVertexBindingRate(SrcBinding.BindingRate);
                VertexInputBindings[i].stride = SrcBinding.Stride;
            }

            for (uint32 i = 0; i < InitInfo.PipelineDesc->VertexLayout.Attributes.size(); ++i)
            {
                auto& SrcAttribute = InitInfo.PipelineDesc->VertexLayout.Attributes[i];

                VertexInputAttributes[i].binding = SrcAttribute.BindingIndex;
                VertexInputAttributes[i].location = SrcAttribute.SlotIndex;
                VertexInputAttributes[i].format = ConvertFormatType(SrcAttribute.Format);
                VertexInputAttributes[i].offset = SrcAttribute.Offset;
            }

            VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            VertexInputInfo.vertexBindingDescriptionCount = (uint32)InitInfo.PipelineDesc->VertexLayout.Bindings.size();
            VertexInputInfo.pVertexBindingDescriptions = VertexInputBindings;
            VertexInputInfo.vertexAttributeDescriptionCount = (uint32)InitInfo.PipelineDesc->VertexLayout.Attributes.size();
            VertexInputInfo.pVertexAttributeDescriptions = VertexInputAttributes;
        }
        Info.pVertexInputState = &VertexInputInfo;

        VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo{};
        InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        InputAssemblyInfo.topology = ConvertVertexBindingRate(InitInfo.PipelineDesc->PrimitiveTopology);
        InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
        Info.pInputAssemblyState = &InputAssemblyInfo;

        // Make viewport is dynamic state so don't need to be initialized at creation
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
        const CRasterizerState* RastState = InitInfo.Backend->GetRenderer()->GetRasterizerState(InitInfo.PipelineDesc->Rasterizer);
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

        VkPipelineMultisampleStateCreateInfo Multisampling{};
        Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        Multisampling.sampleShadingEnable = VK_FALSE;
        Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        Multisampling.minSampleShading = 1.f;
        Multisampling.pSampleMask = nullptr;
        Multisampling.alphaToCoverageEnable = VK_FALSE;
        Multisampling.alphaToOneEnable = VK_FALSE;
        Info.pMultisampleState = &Multisampling;

        VkPipelineDepthStencilStateCreateInfo DepthStencilInfo{};
        const CDepthStencilState* DepthState = InitInfo.Backend->GetRenderer()->GetDepthStencilState(InitInfo.PipelineDesc->DepthStencil);
        DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        DepthStencilInfo.depthTestEnable = DepthState->DepthTestEnable;
        DepthStencilInfo.depthWriteEnable = DepthState->DepthWriteEnable;
        DepthStencilInfo.depthCompareOp = ConvertOperationMode(DepthState->DepthCompareOp);
        DepthStencilInfo.depthBoundsTestEnable = DepthState->DepthBoundsTestEnable;
        DepthStencilInfo.minDepthBounds = DepthState->MinDepthBounds;
        DepthStencilInfo.maxDepthBounds = DepthState->MaxDepthBounds;
        DepthStencilInfo.stencilTestEnable = DepthState->StencilTestEnable;
        DepthStencilInfo.front = {}; // #todo_vk
        DepthStencilInfo.back = {};
        Info.pDepthStencilState = &DepthStencilInfo;

        const CBlendState* BlendState = InitInfo.Backend->GetRenderer()->GetBlendState(InitInfo.PipelineDesc->Blend);
        VkPipelineColorBlendAttachmentState ColorBlendAttachments[10];
        VkPipelineColorBlendStateCreateInfo ColorBlending{};
        ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        ColorBlending.logicOpEnable = VK_FALSE;
        ColorBlending.logicOp = VK_LOGIC_OP_COPY;
        ColorBlending.blendConstants[0] = 0.f;
        ColorBlending.blendConstants[1] = 0.f;
        ColorBlending.blendConstants[2] = 0.f;
        ColorBlending.blendConstants[3] = 0.f;
        ColorBlending.attachmentCount = InitInfo.RenderPass->GetColorAttachmentCount();
        ColorBlending.pAttachments = ColorBlendAttachments;

        for (uint32 i = 0; i < InitInfo.RenderPass->GetColorAttachmentCount(); ++i)
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

        Info.renderPass = InitInfo.RenderPass->Get();
        Info.subpass = 0;

        Result = vkCreateGraphicsPipelines(Device, nullptr, 1, &Info, nullptr, &m_Pipeline);
    }
    else if (InitInfo.PipelineDesc->Type == PipelineType::Compute)
    {
        CASSERT(nullptr == InitInfo.PipelineDesc->PixelShader && nullptr == InitInfo.PipelineDesc->VertexShader);
        m_BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

        VkComputePipelineCreateInfo ComputeInfo{};
        ComputeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        ComputeInfo.flags = Flags;
        ComputeInfo.layout = m_Layout;

        if (InitInfo.PipelineDesc->ComputeShader)
            FillShader(InitInfo.PipelineDesc->VertexShader.get(), ComputeInfo.stage, VK_SHADER_STAGE_COMPUTE_BIT);

        Result = vkCreateComputePipelines(Device, nullptr, 1, &ComputeInfo, nullptr, &m_Pipeline);
    }
    else
    {
        CASSERT(false);
    }

    C_ASSERT_VK_SUCCEEDED_RET(Result, C_STATUS::C_STATUS_ERROR);
    
    return C_STATUS::C_STATUS_OK;
}

void PipelineStateVk::DeInit()
{
    if (m_Pipeline)
    {
        VkDevice Device = m_RenderPass->GetBackend()->GetGlobalContext().GetLogicalDevice(m_RenderPass->GetDevice()).LogicalDeviceHandle;
        vkDestroyPipeline(Device, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(Device, m_Layout, nullptr);

        m_RenderPass = nullptr;
        m_Pipeline = VK_NULL_HANDLE;
        m_Layout = nullptr;
    }
}

C_STATUS PipelineStateVk::Bind(CommandBufferVulkan* CommandBufferVk)
{
    vkCmdBindPipeline(CommandBufferVk->Get(), m_BindPoint, m_Pipeline);

    // #todo_vk_first
    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = 1920.f;
    viewport.height = 1080.f;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(CommandBufferVk->Get(), 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { 1920, 1080 };
    vkCmdSetScissor(CommandBufferVk->Get(), 0, 1, &scissor);

    return C_STATUS::C_STATUS_OK;
}

} //namespace Cyclone::Render
