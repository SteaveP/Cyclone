#include "ShaderVulkan.h"

#include "RenderBackendVulkan.h"
#include "Internal/DeviceManagerVulkan.h"

namespace Cyclone::Render
{

CShaderVulkan::CShaderVulkan() = default;
CShaderVulkan::CShaderVulkan(CShaderVulkan&& Other) noexcept : CShader(MoveTemp(Other))
{
    std::swap(this->m_ShaderModule, Other.m_ShaderModule);
}

CShaderVulkan& CShaderVulkan::operator =(CShaderVulkan&& Other) noexcept
{
    if (this != &Other)
    {
        CShader::operator=(MoveTemp(Other));
        std::swap(this->m_ShaderModule, Other.m_ShaderModule);
    }
    return *this;
}

CShaderVulkan::~CShaderVulkan()
{
    DeInitImpl();

    CASSERT(m_ShaderModule == VK_NULL_HANDLE);
}

C_STATUS CShaderVulkan::Init(const CShaderDesc& Desc)
{
    C_STATUS Result = CShader::Init(Desc);
    C_ASSERT_RETURN_VAL(C_SUCCEEDED(Result), Result);

    CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
    const auto& Device = BackendVk->GetDeviceManager().GetDevice(m_Desc.DeviceHandle);

    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = m_Desc.Bytecode.size();
    CreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_Desc.Bytecode.data());

    VkResult ResultVk = VK_CALL(Device, vkCreateShaderModule(Device.DeviceVk, &CreateInfo, nullptr, &m_ShaderModule));
    C_ASSERT_VK_SUCCEEDED_RET(ResultVk, C_STATUS::C_STATUS_ERROR);

#if ENABLE_DEBUG_RENDER_BACKEND
    if (m_Desc.Name.empty() == false)
    {
        SetDebugNameVk(m_Desc.Name, VK_OBJECT_TYPE_SHADER_MODULE, (uint64)m_ShaderModule, Device);
    }
#endif

    return C_STATUS::C_STATUS_OK;
}

void CShaderVulkan::DeInit()
{
    DeInitImpl();
    CShader::DeInit();
}

void CShaderVulkan::DeInitImpl()
{
    if (m_ShaderModule)
    {
        CRenderBackendVulkan* BackendVk = GET_BACKEND_IMPL(m_Desc.Backend);
        BackendVk->GetDisposalManagerVk(m_Desc.DeviceHandle)->AddDisposable([Backend = BackendVk, DeviceHandle = m_Desc.DeviceHandle, Shader = m_ShaderModule]()
        {
            const auto& Device = Backend->GetDeviceManager().GetDevice(DeviceHandle);
            VK_CALL(Device, vkDestroyShaderModule(Device.DeviceVk, Shader, nullptr));
        });

        m_ShaderModule = VK_NULL_HANDLE;
    }
}

} // namespace Cyclone::Render
