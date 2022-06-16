#include "Buffer.h"

namespace Cyclone::Render
{

CBuffer::CBuffer() = default;
CBuffer::~CBuffer() = default;

Cyclone::C_STATUS CBuffer::Init(const CBufferDesc& Desc)
{
    m_Desc = Desc;
    return C_STATUS::C_STATUS_OK;
}

void CBuffer::DeInit()
{

}

} // namespace Cyclone::Render
