#include "Texture.h"

namespace Cyclone::Render
{

CTexture::CTexture() = default;
CTexture::~CTexture() = default;

Cyclone::C_STATUS CTexture::Init(const CTextureDesc& Desc)
{
    m_Desc = Desc;
    return C_STATUS::C_STATUS_OK;
}

void CTexture::DeInit()
{

}

} // namespace Cyclone::Render
