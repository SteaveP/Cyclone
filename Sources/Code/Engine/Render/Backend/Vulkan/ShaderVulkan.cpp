#include "ShaderVulkan.h"

#include "RenderBackendVulkan.h"
#include "Internal/GlobalContextVk.h"

namespace Cyclone::Render
{

CShaderVulkan::CShaderVulkan() = default;
CShaderVulkan::~CShaderVulkan() = default;

C_STATUS CShaderVulkan::Init(const CShaderDesc& Desc)
{
    C_STATUS Result = CShader::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    RenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
    VkDevice Device = BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.Device).LogicalDeviceHandle;

    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = m_Desc.Bytecode.size();
    CreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_Desc.Bytecode.data());

    VkResult ResultVk = vkCreateShaderModule(Device, &CreateInfo, nullptr, &m_ShaderModule);
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

    return C_STATUS::C_STATUS_OK;
}

void CShaderVulkan::DeInit()
{
    if (m_ShaderModule)
    {
        RenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
        VkDevice Device = BackendVk->GetGlobalContext().GetLogicalDevice(m_Desc.Device).LogicalDeviceHandle;

        vkDestroyShaderModule(Device, m_ShaderModule, nullptr);
        m_ShaderModule = VK_NULL_HANDLE;
    }

    CShader::DeInit();
}

} // namespace Cyclone::Render
