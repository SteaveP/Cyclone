#pragma once

#include "Engine/Render/Backend/Shader.h"
#include "CommonVulkan.h"

namespace Cyclone::Render
{
   
class CShaderVulkan : public CShader
{
public:
    DISABLE_COPY_ENABLE_MOVE_DECL(CShaderVulkan);

    CShaderVulkan();
    virtual ~CShaderVulkan();

    virtual C_STATUS Init(const CShaderDesc& Desc) override;
    virtual void DeInit() override;

    virtual void* GetBackendDataPtr() override { return m_ShaderModule; }

    const CShaderDesc& GetDesc() const { return m_Desc; }

private:
    void DeInitImpl();

protected:
    VkShaderModule m_ShaderModule = VK_NULL_HANDLE;
};

} // namespace Cyclone::Render
