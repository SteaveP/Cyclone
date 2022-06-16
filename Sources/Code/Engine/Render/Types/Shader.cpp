#include "Shader.h"

namespace Cyclone::Render
{

CShader::CShader() = default;
CShader::~CShader() = default;

Cyclone::C_STATUS CShader::Init(const CShaderDesc& Desc)
{
    m_Desc = Desc;
    return C_STATUS::C_STATUS_OK;
}

void CShader::DeInit()
{

}

} // namespace Cyclone::Render
