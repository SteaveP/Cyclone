#pragma once

#include "Engine/Render/CommonRender.h"

namespace Cyclone::Render
{

enum class EShaderType
{
    Vertex,
    Pixel,
    Compute,
    Count
};

class ENGINE_API CShaderDesc
{
public:
    EShaderType Type;

    Vector<char> Bytecode;
    String EntryPoint;

    IRendererBackend* Backend = nullptr;
    CDeviceHandle DeviceHandle;

#if ENABLE_DEBUG_RENDER_BACKEND
    String Name;
#endif
};

class ENGINE_API CShader
{
public:
    DISABLE_COPY_ENABLE_MOVE(CShader);

    CShader();
    virtual ~CShader();

    virtual C_STATUS Init(const CShaderDesc& Desc);
    virtual void DeInit() {};

    virtual void* GetBackendDataPtr() = 0;

    const CShaderDesc& GetDesc() const { return m_Desc; }

protected:
    CShaderDesc m_Desc; // #todo_vk remove bytecode to free up memory and store data to be able to hot reload shaders
};

} // namespace Cyclone::Render
