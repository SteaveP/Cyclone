#pragma once

#include "Engine/Framework/IRenderer.h"
#include "Engine/Render/IRendererBackend.h"

#include "CommonDX12.h"

// #todo_dx12 refactor
#include "Engine/Render/Types/WindowContext.h"

namespace Cyclone::Render
{


// #todo_dx12 refactor
class WindowContextDX12 : public CWindowContext
{
public:
    CCommandQueue* GetCommandQueue(CommandQueueType QueueType) const override { return nullptr; }

};

class RenderBackendDX12 : public IRendererBackend
{
public:
    virtual C_STATUS Init(IRenderer* Renderer) override;
    virtual C_STATUS Shutdown() override;

    virtual C_STATUS BeginRender() override;
    virtual C_STATUS EndRender() override;

    IRenderer* GetRenderer() const { return m_Renderer; }

    virtual CWindowContext* CreateWindowContext(IWindow* Window) override;
    virtual CCommandQueue* CreateCommandQueue() override;
    virtual CCommandBuffer* CreateCommandBuffer() override;
    virtual CTexture* CreateTexture() override;

protected:
    IRenderer* m_Renderer = nullptr;

protected:

};

} // namespace Cyclone::Render
