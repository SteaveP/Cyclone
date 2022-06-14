#pragma once

#include "Engine/EngineModule.h"
#include "Engine/Framework/IRenderer.h"

#include "Engine/Render/Common.h"

namespace Cyclone::Render
{

class ENGINE_API CCommandContext
{
public:
    DISABLE_COPY_ENABLE_MOVE(CCommandContext);

    CCommandContext() = default;
    virtual ~CCommandContext() = default;

    virtual C_STATUS Init(IRenderer* Renderer, CCommandBuffer* CommandBuffer);
    virtual void DeInit();

    virtual void BeginRenderPass(CCommandBuffer* CommandBuffer, const CRenderPass& RenderPass);
    virtual void EndRenderPass(CCommandBuffer* CommandBuffer);

protected:
    IRenderer* m_Renderer = nullptr;
    CCommandBuffer* m_CommandBuffer = nullptr;
    CRenderPass m_RenderPass;
};

class ENGINE_API CCommandBuffer
{
public:
    DISABLE_COPY_ENABLE_MOVE(CCommandBuffer);

    CCommandBuffer();
    virtual ~CCommandBuffer();

    virtual void DeInit();

    virtual C_STATUS Begin();
    virtual void End();

    // Set State commands
    virtual void SetRenderTargets(CRenderTargetSet RenderTargetSet);

    virtual void BeginRenderPass(const CRenderPass& RenderPass);
    virtual void EndRenderPass();
 
    // Draw commands
    virtual void Draw();

    // Dispatch commands
    virtual void Dispatch();

    // Copy commands
    virtual void Copy();

    // Clear commands
    virtual void Clear();

    // Indirect commands
    // #todo_vk

protected:
    UniquePtr<CCommandContext> m_Context;
};

} // namespace Cyclone::Render
