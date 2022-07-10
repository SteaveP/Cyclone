#include "SceneRenderer.h"

#include "Engine/Framework/IRenderer.h"
#include "Engine/Framework/IApplication.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneViewport.h"
#include "Engine/Utils/Profiling.h"

#include "RenderScene.h"
#include "RenderSceneView.h"

#include "Engine/Render/Backend/IRendererBackend.h"
#include "Engine/Render/Backend/IBindlessManager.h"
#include "Engine/Render/Backend/IResourceManager.h"
#include "Engine/Render/Backend/UploadQueue.h"
#include "Engine/Render/Backend/WindowContext.h"
#include "Engine/Render/Backend/CommandQueue.h"
#include "Engine/Render/Backend/CommandBuffer.h"
#include "Engine/Render/Backend/Resource.h"
#include "Engine/Render/Backend/ResourceView.h"
#include "Engine/Render/Backend/Shader.h"
#include "Engine/Render/Backend/Pipeline.h"

#include "Engine/Render/Mesh.h"

#include <iostream>
#include <fstream>

// #todo_vk_resource #todo_vk_material #todo_asset fixme
#define TINYGLTF_USE_CPP14
#define TINYGLTF_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/ImGuiFileDialog/stb/stb_image.h"

#define CONVERT_FOR_HLSL(Offset) ((Offset) / 4)

static std::vector<char> ReadFile(const std::string& filename);

namespace Cyclone::Render
{

constexpr uint32 GAlignment = 16;

struct alignas(GAlignment) MaterialDataHLSL
{
    Vec3 Color;
    uint32 TextureIndex;
};
static_assert(sizeof(MaterialDataHLSL) >= GAlignment);

struct alignas(GAlignment) InstanceDataHLSL
{
    Mat44 WorldMatrix;
    uint32 MaterialIndex;
};

struct alignas(GAlignment) InstanceDataBindlessHLSL
{
    // offsets in 4 bytes (uint indexing)
    uint32 IndexBufferOffset = 0; 
    uint32 VertexBufferOffset = 0;
    uint32 InstanceDataOffset = 0;
    // #todo_vk offsets for each vert buffer binding?
};

// #todo_vk #todo_refactor remove this
static bool IsInit = false;
#define TEST_UPLOAD_DATA 1

struct CUnitTest
{
    static const uint32 InstanceCount = 4;
    static const uint32 MaterialCount = 2;

    CAllocHandle IndexBuffer;
    CAllocHandle VertexBuffer;
    CAllocHandle InstanceBuffer; // Intended to be set as vertex buffer, contains offset to InstanceBindlessDataBuffer for each instance

    CAllocHandle InstanceBindlessDataBuffer; // Offsets to Index, Vertex and Instance data
    CAllocHandle InstanceDataBuffer; // Matrices and mat index
    CAllocHandle MaterialDataBuffer; // Color and textures

    CHandle<CShader> VertexShader;
    CHandle<CShader> PixelShader;

    CHandle<CShader> FullScreenQuadVertexShader;
    CHandle<CShader> CheckerboardPixelShader;
    CHandle<CShader> ProceduralPixelShader;

    CPipelineStateDesc PipelineDesc{};
    CHandle<CPipelineState> Pipeline;

    CPipelineStateDesc FillTexPipelineDesc{};
    CHandle<CPipelineState> FillTexPipeline;
    CPipelineStateDesc FillTex2PipelineDesc{};
    CHandle<CPipelineState> FillTex2Pipeline;

    CShaderBindingSet LayoutDesc{};

    //Vector<Material> Materials;

    // #todo_vk Shader 
    // #todo_vk render graph
    Array<CRenderTarget, 2> Tex;
    Array<uint32, 2> TexBindlessHandles;

    CRenderTarget DepthTex;

    CHandle<CResource> LoadedTex;
    CHandle<CResourceView> LoadedTextSRV;
    uint32 LoadedTexBindlessHandle = -1;

    void DeInit(IRenderer* Renderer);
};
Ptr<CUnitTest> GTest;

void GUnitTestInit(IRendererBackend* Backend, CDeviceHandle DeviceHandle)
{
    static bool IsInit = false;
    if (IsInit)
        return;

    GTest = MakeShared<CUnitTest>();

    IsInit = true;

    IBindlessManager* BindlessManager = Backend->GetBindlessManager(DeviceHandle);
    CASSERT(BindlessManager);
    IResourceManager* ResourceManager = Backend->GetResourceManager(DeviceHandle);
    CASSERT(ResourceManager);
    CUploadQueue* UploadQueue = Backend->GetUploadQueue(DeviceHandle);
    CASSERT(UploadQueue);

    {
        auto& Layouts = GTest->LayoutDesc.SetLayouts;
        // #todo_vk fill layout specific to mesh/shader
        ResourceManager->AddSystemShaderBindingsTo(GTest->LayoutDesc);
    }

    {
        auto& ShH = GTest->VertexShader;
        ShH = ResourceManager->CreateShader();

        auto Sh = ResourceManager->GetShader(ShH);
        CShaderDesc Desc{};
        Desc.Backend = Backend;
        Desc.DeviceHandle = DeviceHandle;
        Desc.Type = EShaderType::Vertex;
        Desc.EntryPoint = "main";
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "VertexShader";
#endif
        Desc.Bytecode = ReadFile("Intermediate/Shaders/Vert.spv");

        C_STATUS Result = Sh->Init(Desc);
        C_ASSERT_RETURN(C_SUCCEEDED(Result));

        //
        Desc.Bytecode = ReadFile("Intermediate/Shaders/FullScreenVert.spv");
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "FullScreenVertexShader";
#endif
        GTest->FullScreenQuadVertexShader = ResourceManager->CreateShader();

        Result = ResourceManager->GetShader(GTest->FullScreenQuadVertexShader)->Init(Desc);
        C_ASSERT_RETURN(C_SUCCEEDED(Result));
    }
    {
        auto& ShH = GTest->PixelShader;
        ShH = ResourceManager->CreateShader();

        auto Sh = ResourceManager->GetShader(ShH);

        CShaderDesc Desc{};
        Desc.Backend = Backend;
        Desc.DeviceHandle = DeviceHandle;
        Desc.Type = EShaderType::Pixel;
        Desc.EntryPoint = "main";
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "PixelShader";
#endif
        Desc.Bytecode = ReadFile("Intermediate/Shaders/Frag.spv");

        C_STATUS Result = Sh->Init(Desc);
        C_ASSERT_RETURN(C_SUCCEEDED(Result));

        // 1
        Desc.Bytecode = ReadFile("Intermediate/Shaders/TexFillFrag.spv");
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "CheckerboardPixelShader";
#endif
        GTest->CheckerboardPixelShader = ResourceManager->CreateShader();

        Result = ResourceManager->GetShader(GTest->CheckerboardPixelShader)->Init(Desc);
        C_ASSERT_RETURN(C_SUCCEEDED(Result));

        // 2
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "ProceduralPixelShader";
#endif
        Desc.Bytecode = ReadFile("Intermediate/Shaders/TexFill2Frag.spv");
        GTest->ProceduralPixelShader = ResourceManager->CreateShader();

        Result = ResourceManager->GetShader(GTest->ProceduralPixelShader)->Init(Desc);
        C_ASSERT_RETURN(C_SUCCEEDED(Result));
    }

    {
        alignas(GAlignment) uint16 Indices[] = { 0, 1, 2,  2, 1, 3 };

        auto& H = GTest->IndexBuffer;
        CAllocDesc ADesc{};
        ADesc.DeviceHandle = DeviceHandle;
        ADesc.ByteCount = sizeof(Indices);
        H = BindlessManager->AllocatePersistent(ADesc);

#if TEST_UPLOAD_DATA
        C_STATUS Result = UploadQueue->QueueUploadData(Indices, sizeof(Indices),
            H.Buffer, H.GetDeviceMemBufferByteOffset());
        CASSERT(C_SUCCEEDED(Result));
#else
        CResource* Resource = ResourceManager->GetResource(H.Buffer);
        CASSERT(Resource);

        if (CMapData MD = Resource->Map())
        {
            memcpy((uint8*)MD.Memory + H.HostBufferByteOffset, Indices, sizeof(Indices));
            Resource->UnMap(MD);
        }
#endif
    }

    {
        VertexPosColorUV VertexData[4] = {
            VertexPosColorUV {.Pos = Vec3{-0.00f, -0.25f, 0.25f}, .Color = Vec3 {1.f, 0.f, 0.f}, .TexCoord = Vec2 {0.5f, 0.f}},
            VertexPosColorUV {.Pos = Vec3{ 0.25f,  0.25f, 0.25f}, .Color = Vec3 {0.f, 0.f, 1.f}, .TexCoord = Vec2 {1.0f, 1.f}},
            VertexPosColorUV {.Pos = Vec3{-0.25f,  0.25f, 0.25f}, .Color = Vec3 {0.f, 1.f, 0.f}, .TexCoord = Vec2 {0.0f, 1.f}},
            VertexPosColorUV {.Pos = Vec3{-0.00f,  0.75f, 0.25f}, .Color = Vec3 {.3f, .3f, .3f}, .TexCoord = Vec2 {0.5f, 0.f}},
        };

        auto& H = GTest->VertexBuffer;
        CAllocDesc ADesc{};
        ADesc.DeviceHandle = DeviceHandle;
        ADesc.ByteCount = sizeof(VertexData);
        H = BindlessManager->AllocatePersistent(ADesc);

#if TEST_UPLOAD_DATA
        C_STATUS Result = UploadQueue->QueueUploadData(VertexData, sizeof(VertexData),
            H.Buffer, H.GetDeviceMemBufferByteOffset());
        CASSERT(C_SUCCEEDED(Result));
#else
        CResource* Resource = ResourceManager->GetResource(H.Buffer);
        CASSERT(Resource);

        if (CMapData MD = Resource->Map())
        {
            memcpy((uint8*)MD.Memory + H.HostBufferByteOffset, &VertexData[0], sizeof(VertexData));
            Resource->UnMap(MD);
        }
#endif
    }

    for (int i = 0; i < GTest->Tex.size(); ++i)
    {
        auto& RT = GTest->Tex[i];

        const auto& BackBuffer = ResourceManager->GetResource(Backend->GetRenderer()->GetDefaultWindowContext()->GetBackBuffer(0).Texture);

        CResourceDesc Desc{};
        Desc.DeviceHandle = DeviceHandle;
        Desc.Backend = Backend;
        Desc.Format = BackBuffer->GetDesc().Format;
        Desc.Flags = EResourceFlags::Texture;
        Desc.InitialLayout = EImageLayoutType::Undefined;
        Desc.Texture.InitialUsage = EImageUsageType::None;
        Desc.Texture.ImageType = ETextureType::Type2D;
        Desc.Texture.Usage = EImageUsageType::ColorAttachment | EImageUsageType::ShaderResourceView;
        Desc.Texture.Width = BackBuffer->GetDesc().Texture.Width;
        Desc.Texture.Height = BackBuffer->GetDesc().Texture.Height;
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "MyTexture " + ToString(i);
#endif

        RT.Texture = ResourceManager->CreateResource(Desc);
        C_ASSERT_RETURN(RT.Texture.IsValid());

        CResourceViewDesc ViewDesc{};
        ViewDesc.Backend = Backend;
        ViewDesc.Type = EResourceFlags::Texture;
        ViewDesc.Resource = RT.Texture;
        ViewDesc.Format = Desc.Format;
        ViewDesc.Texture.ViewType = ETextureViewType::Type2D;
        ViewDesc.Texture.AspectMask = EImageAspectType::Color;
        RT.RenderTargetView = ResourceManager->CreateResourceView(ViewDesc);
        C_ASSERT_RETURN(RT.RenderTargetView.IsValid());

        GTest->TexBindlessHandles[i] = BindlessManager->RegisterResource(RT.RenderTargetView);
    }

    {
        auto& T = GTest->LoadedTex;
        CResourceDesc Desc{};
        Desc.DeviceHandle = DeviceHandle;
        Desc.Backend = Backend;
        Desc.Format = EFormatType::RGBA8_SRGB;
        Desc.Flags = EResourceFlags::Texture;
        Desc.InitialLayout = EImageLayoutType::Undefined;
        Desc.Texture.InitialUsage = EImageUsageType::None;
        Desc.Texture.ImageType = ETextureType::Type2D;
        Desc.Texture.Usage = EImageUsageType::TransferDst | EImageUsageType::ShaderResourceView;
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "LoadedTexture";
#endif

        int32 x, y, comp;
        uint8* Data = stbi_load("Sources/Assets/Textures/Test.png", &x, &y, &comp, 4);
        CASSERT(Data);
        if (Data)
        {
            Desc.Texture.Width = x;
            Desc.Texture.Height = y;

            T = ResourceManager->CreateResource(Desc);
            C_ASSERT_RETURN(T.IsValid());

            CResource* Texture = ResourceManager->GetResource(T);
            C_ASSERT_RETURN(Texture);
#if TEST_UPLOAD_DATA
            C_STATUS Result = UploadQueue->QueueUploadData(Data, x * y * sizeof(uint32), T, 0, 
                EImageLayoutType::ShaderReadOnly, EImageUsageType::ShaderResourceView, 256u, 0, 1);
            CASSERT(C_SUCCEEDED(Result));
#else
            #error do something
#endif
            CResourceViewDesc ViewDesc{};
            ViewDesc.Backend = Backend;
            ViewDesc.Type = EResourceFlags::Texture;
            ViewDesc.Resource = GTest->LoadedTex;
            ViewDesc.Format = Desc.Format;

            GTest->LoadedTextSRV = ResourceManager->CreateResourceView(ViewDesc);
            CASSERT(GTest->LoadedTextSRV.IsValid());

            GTest->LoadedTexBindlessHandle = BindlessManager->RegisterResource(GTest->LoadedTextSRV);
            CASSERT(GTest->LoadedTexBindlessHandle != ~0u);

            stbi_image_free(Data);
        }
    }

    {
        auto& RT = GTest->DepthTex;

        const auto& BackBuffer = ResourceManager->GetResource(Backend->GetRenderer()->GetDefaultWindowContext()->GetBackBuffer(0).Texture);

        CResourceDesc Desc{};
        Desc.DeviceHandle = DeviceHandle;
        Desc.Backend = Backend;
        Desc.Format = EFormatType::D_32;
        Desc.Flags = EResourceFlags::Texture;
        Desc.InitialLayout = EImageLayoutType::Undefined;
        Desc.Texture.InitialUsage = EImageUsageType::None;
        Desc.Texture.ImageType = ETextureType::Type2D;
        Desc.Texture.Usage = EImageUsageType::DepthStencil | EImageUsageType::DepthStencilRead;
        Desc.Texture.Width = BackBuffer->GetDesc().Texture.Width;
        Desc.Texture.Height = BackBuffer->GetDesc().Texture.Height;
#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "SceneDepth";
#endif

        RT.Texture = ResourceManager->CreateResource(Desc);
        C_ASSERT_RETURN(RT.Texture.IsValid());

        CResourceViewDesc ViewDesc{};
        ViewDesc.Backend = Backend;
        ViewDesc.Type = EResourceFlags::Texture;
        ViewDesc.Resource = RT.Texture;
        ViewDesc.Format = Desc.Format;
        ViewDesc.Texture.ViewType = ETextureViewType::Type2D; 
        ViewDesc.Texture.AspectMask = EImageAspectType::Depth;
        RT.ShaderResourceView = ResourceManager->CreateResourceView(ViewDesc);
        C_ASSERT_RETURN(RT.ShaderResourceView.IsValid());

        ViewDesc.Texture.AspectMask = EImageAspectType::Depth;
#if ENABLE_DEBUG_RENDER_BACKEND
        ViewDesc.Name = Desc.Name + " DSV";
#endif
        RT.DepthStencilView = ResourceManager->CreateResourceView(ViewDesc);
        C_ASSERT_RETURN(RT.DepthStencilView.IsValid());
    }

    {
        MaterialDataHLSL Data[CUnitTest::MaterialCount]{};

        Data[0].Color = Vec3(0.3f, 0.88f, 0.25f);
        Data[0].TextureIndex = GTest->TexBindlessHandles[0];

        Data[1].Color = Vec3(0.2f, 0.48f, 0.85f);
#if 1
        Data[1].TextureIndex = GTest->LoadedTexBindlessHandle;
#else
        Data[1].TextureIndex = GTest->GTest->TexBindlessHandles[1];
#endif

        auto& H = GTest->MaterialDataBuffer;
        CAllocDesc ADesc{};
        ADesc.DeviceHandle = DeviceHandle;
        ADesc.ByteCount = sizeof(MaterialDataHLSL) * CUnitTest::MaterialCount;
        H = BindlessManager->AllocatePersistent(ADesc);
        CASSERT(H.Buffer.IsValid());

#if TEST_UPLOAD_DATA
        C_STATUS Result = UploadQueue->QueueUploadData(Data, sizeof(Data),
            H.Buffer, H.GetDeviceMemBufferByteOffset());
        CASSERT(C_SUCCEEDED(Result));
#else
        CResource* Resource = ResourceManager->GetResource(H.Buffer);
        CASSERT(Resource);

        if (CMapData MD = Resource->Map())
        {
            memcpy((uint8*)MD.Memory + H.HostBufferByteOffset, Data, sizeof(Data));
            Resource->UnMap(MD);
        }
#endif
    }

    {
        InstanceDataHLSL InstData[CUnitTest::InstanceCount]{};

        InstData[0].WorldMatrix.X = Vec4{ -0.2f, -0.2f, 0.f };
        InstData[0].WorldMatrix.Z = Vec4{ -0.3f, -0.3f, 0.f };
        InstData[0].MaterialIndex = (uint32)GTest->MaterialDataBuffer.DeviceMemBufferDWORDOffset + CONVERT_FOR_HLSL(0 * sizeof(MaterialDataHLSL));

        InstData[1].WorldMatrix.X = Vec4{ 0.2f, -0.2f, 0.f };
        InstData[1].MaterialIndex = (uint32)GTest->MaterialDataBuffer.DeviceMemBufferDWORDOffset + CONVERT_FOR_HLSL(1 * sizeof(MaterialDataHLSL));

        InstData[2].WorldMatrix.X = Vec4{ -0.2f, 0.2f, 0.f };
        InstData[2].MaterialIndex = (uint32)GTest->MaterialDataBuffer.DeviceMemBufferDWORDOffset + CONVERT_FOR_HLSL(1 * sizeof(MaterialDataHLSL));

        InstData[3].WorldMatrix.X = Vec4{ 0.2f,  0.2f, 0.f };
        InstData[3].MaterialIndex = (uint32)GTest->MaterialDataBuffer.DeviceMemBufferDWORDOffset + CONVERT_FOR_HLSL(0 * sizeof(MaterialDataHLSL));

        auto& H = GTest->InstanceDataBuffer;
        CAllocDesc ADesc{};
        ADesc.DeviceHandle = DeviceHandle;
        ADesc.ByteCount = sizeof(InstanceDataHLSL) * CUnitTest::InstanceCount;
        H = BindlessManager->AllocatePersistent(ADesc);
        CASSERT(H.Buffer.IsValid());

#if TEST_UPLOAD_DATA
        C_STATUS Result = UploadQueue->QueueUploadData(InstData, sizeof(InstData),
            H.Buffer, H.GetDeviceMemBufferByteOffset());
        CASSERT(C_SUCCEEDED(Result));
#else
        CResource* Resource = ResourceManager->GetResource(H.Buffer);
        CASSERT(Resource);

        if (CMapData MD = Resource->Map())
        {
            memcpy((uint8*)MD.Memory + H.HostBufferByteOffset, InstData, sizeof(InstData));
            Resource->UnMap(MD);
        }
#endif
    }

    {
        InstanceDataBindlessHLSL Data[CUnitTest::InstanceCount]{};

        for (uint32 i = 0; i < GTest->InstanceCount; ++i)
        {
            Data[i].IndexBufferOffset = (uint32)GTest->IndexBuffer.DeviceMemBufferDWORDOffset;
            Data[i].VertexBufferOffset = (uint32)GTest->VertexBuffer.DeviceMemBufferDWORDOffset;
            Data[i].InstanceDataOffset = (uint32)GTest->InstanceDataBuffer.DeviceMemBufferDWORDOffset + CONVERT_FOR_HLSL(i * sizeof(InstanceDataHLSL));
        }

        auto& H = GTest->InstanceBindlessDataBuffer;
        CAllocDesc ADesc{};
        ADesc.DeviceHandle = DeviceHandle;
        ADesc.ByteCount = sizeof(InstanceDataBindlessHLSL) * CUnitTest::InstanceCount;
        H = BindlessManager->AllocatePersistent(ADesc);
        CASSERT(H.Buffer.IsValid());

#if TEST_UPLOAD_DATA
        C_STATUS Result = UploadQueue->QueueUploadData(Data, sizeof(Data),
            H.Buffer, H.GetDeviceMemBufferByteOffset());
        CASSERT(C_SUCCEEDED(Result));
#else
        CResource* Resource = ResourceManager->GetResource(H.Buffer);
        CASSERT(Resource);

        if (CMapData MD = Resource->Map())
        {
            memcpy((uint8*)MD.Memory + H.HostBufferByteOffset, Data, sizeof(Data));
            Resource->UnMap(MD);
        }
#endif
    }

    {
        uint32 Data[CUnitTest::InstanceCount];
        for (uint32 i = 0; i < CUnitTest::InstanceCount; ++i)
            Data[i] = static_cast<uint32>(GTest->InstanceBindlessDataBuffer.DeviceMemBufferDWORDOffset) + CONVERT_FOR_HLSL(i * sizeof(InstanceDataBindlessHLSL));

        auto& H = GTest->InstanceBuffer;
        CAllocDesc ADesc{};
        ADesc.DeviceHandle = DeviceHandle;
        ADesc.ByteCount = sizeof(uint32) * CUnitTest::InstanceCount;
        H = BindlessManager->AllocatePersistent(ADesc);

#if TEST_UPLOAD_DATA
        C_STATUS Result = UploadQueue->QueueUploadData(Data, sizeof(Data),
            H.Buffer, H.GetDeviceMemBufferByteOffset());
        CASSERT(C_SUCCEEDED(Result));
#else
        CResource* Resource = ResourceManager->GetResource(H.Buffer);
        CASSERT(Resource);

        if (CMapData MD = Resource->Map())
        {
            memcpy((uint8*)MD.Memory + H.HostBufferByteOffset, Data, sizeof(Data));
            Resource->UnMap(MD);
        }
#endif
    }

    // Pipeline
    {
        CPipelineStateDesc& Desc = GTest->PipelineDesc;
        Desc = CPipelineStateDesc{};
        Desc.Backend = Backend;
        Desc.DeviceHandle = DeviceHandle;
        Desc.Type = PipelineType::Graphics;
        Desc.DepthStencil = Backend->GetRenderer()->DepthStencilDepthWrite;
        Desc.Blend = Backend->GetRenderer()->BlendDisabled;
        Desc.Rasterizer = Backend->GetRenderer()->RasterizerDefault;
        Desc.PrimitiveTopology = PrimitiveTopologyType::TriangleList;
        Desc.VertexLayout = VertexPosColorUV::GetLayoutDescription();
        ResourceManager->AddSystemVertexBindingsTo(Desc.VertexLayout);

        Desc.ShaderBindingSet = GTest->LayoutDesc;
        Desc.VertexShader = GTest->VertexShader;
        Desc.PixelShader = GTest->PixelShader;

        Desc.RenderTargets.resize(1);
        const auto& DepthBuffer = ResourceManager->GetResource(GTest->DepthTex.Texture);
        const auto& BackBuffer = ResourceManager->GetResource(Backend->GetRenderer()->GetDefaultWindowContext()->GetBackBuffer(0).Texture);

        Desc.RenderTargets[0] = BackBuffer->GetDesc().Format;
        Desc.DepthTarget = DepthBuffer->GetDesc().Format;

#if ENABLE_DEBUG_RENDER_BACKEND
        Desc.Name = "DefaultMeshPipeline";
#endif

        GTest->Pipeline = ResourceManager->GetPipelineStateCached(Desc);
        CASSERT(GTest->Pipeline.IsValid());

        // 1
        GTest->FillTexPipelineDesc = Desc;
        GTest->FillTexPipelineDesc.DepthStencil = Backend->GetRenderer()->DepthStencilDisabled;
        GTest->FillTexPipelineDesc.DepthTarget.reset();
        GTest->FillTexPipelineDesc.VertexShader = GTest->FullScreenQuadVertexShader;
        GTest->FillTexPipelineDesc.PixelShader = GTest->CheckerboardPixelShader;

#if ENABLE_DEBUG_RENDER_BACKEND
        GTest->FillTexPipelineDesc.Name = "FillTexPipeline";
#endif
        GTest->FillTexPipeline = ResourceManager->GetPipelineStateCached(GTest->FillTexPipelineDesc);
        CASSERT(GTest->FillTexPipeline.IsValid());

        // 2
        GTest->FillTex2PipelineDesc = Desc;
        GTest->FillTex2PipelineDesc.DepthStencil = Backend->GetRenderer()->DepthStencilDisabled;
        GTest->FillTex2PipelineDesc.DepthTarget.reset();
        GTest->FillTex2PipelineDesc.VertexShader = GTest->FullScreenQuadVertexShader;
        GTest->FillTex2PipelineDesc.PixelShader = GTest->ProceduralPixelShader;

#if ENABLE_DEBUG_RENDER_BACKEND
        GTest->FillTex2PipelineDesc.Name = "FillTex2Pipeline";
#endif
        GTest->FillTex2Pipeline = ResourceManager->GetPipelineStateCached(GTest->FillTex2PipelineDesc);
        CASSERT(GTest->FillTex2Pipeline.IsValid());
    }
}

void CSceneRenderer::RenderSceneView(CRenderSceneView* SceneView, uint32 SceneViewIndex)
{
    static uint32 ProcessedFrame = 0xFFFFFFFF;

    CCommandQueue* CommandQueue = SceneView->RenderWindowContext->GetCommandQueue(CommandQueueType::Graphics);
    C_ASSERT_RETURN(CommandQueue);

    CCommandBuffer* CommandBuffer = CommandQueue->AllocateCommandBuffer();
    C_ASSERT_RETURN(CommandBuffer);

    C_STATUS Result = CommandBuffer->Begin();
    C_ASSERT_RETURN(C_SUCCEEDED(Result));

    // Draw logic
    {
        // #todo_profiling refactor to: CommmandBuffer->BeginMarker(marker, bool AutoEnd at close) as well as for CommmandQueue
        PROFILE_GPU_SCOPED_EVENT(CommandBuffer, "Scene View %d", SceneViewIndex);

        // Set Render Targets
        CRenderPass RenderPass{};

        RenderPass.ViewportExtent = {
            SceneView->Viewport->UpperLeftCorner.X,     SceneView->Viewport->UpperLeftCorner.Y,
            SceneView->Viewport->BottomRightCorner.X,   SceneView->Viewport->BottomRightCorner.Y,
        };

        CRenderTargetSet& RenderTargetSet = RenderPass.RenderTargetSet;

        RenderTargetSet.RenderTargetsCount = 1;
        auto& RenderTarget = RenderTargetSet.RenderTargets[0];
        RenderTarget.RenderTarget = SceneView->RenderTarget;
        RenderTarget.FinalLayout = EImageLayoutType::ShaderReadOnly;
        RenderTarget.FinalUsage = EImageUsageType::ShaderResourceView;
        RenderTarget.LoadOp = ERenderTargetLoadOp::Clear;
        RenderTarget.StoreOp = ERenderTargetStoreOp::Store;
        RenderTarget.ClearValue = CClearColor(SceneViewIndex % 2 == 0 ? Vec4{ 0.5f, 0.86f, 0.1f, 1.f } : Vec4{ 0.13f, 0.23f, 0.87f, 1.f });

        // Fullscreen quads to fill texture procedurally
        uint32 TexIndex = SceneViewIndex % 2;
        {
            PROFILE_GPU_SCOPED_EVENT(CommandBuffer, "Update Tex %d", TexIndex);

            auto RenderPassCopy = RenderPass;
            RenderPassCopy.RenderTargetSet.RenderTargets[0].ClearValue = CClearColor(TexIndex == 0 ? Vec4{ 0.71f, 0.17f, 0.37f, 1.f } : Vec4{ 0.13f, 0.89f, 0.33f, 1.f });
            RenderPassCopy.RenderTargetSet.RenderTargets[0].RenderTarget = GTest->Tex[TexIndex];
            RenderPassCopy.RenderTargetSet.RenderTargets[0].FinalLayout = EImageLayoutType::ShaderReadOnly;
            RenderPassCopy.RenderTargetSet.RenderTargets[0].FinalUsage = EImageUsageType::ShaderResourceView;

            // Viewport
            IResourceManager* ResourceManager = m_Renderer->GetRendererBackend()->GetResourceManager(CDeviceHandle::From(GTest->Tex[TexIndex].Texture));
            CASSERT(ResourceManager);
            CResource* TexPtr = ResourceManager->GetResource(GTest->Tex[TexIndex].Texture);
            CASSERT(TexPtr);

            RenderPassCopy.ViewportExtent.X = 0;
            RenderPassCopy.ViewportExtent.Y = 0;
            RenderPassCopy.ViewportExtent.Z = (float)TexPtr->GetDesc().Texture.Width;
            RenderPassCopy.ViewportExtent.W = (float)TexPtr->GetDesc().Texture.Height;

            CommandBuffer->BeginRenderPass(RenderPassCopy);

            CommandBuffer->SetPipeline(TexIndex ? GTest->FillTexPipeline : GTest->FillTex2Pipeline);

            CommandBuffer->SetIndexBuffer(GTest->IndexBuffer.Buffer, GTest->IndexBuffer.GetDeviceMemBufferByteOffset(), EFormatType::R16_UINT);
            CommandBuffer->SetVertexBuffer(GTest->VertexBuffer.Buffer, 0, GTest->VertexBuffer.GetDeviceMemBufferByteOffset());
            // don't bother with instanñing here, is isn't used anyway, but should be binded
            CommandBuffer->SetVertexBuffer(GTest->InstanceBuffer.Buffer, 1, GTest->InstanceBuffer.GetDeviceMemBufferByteOffset());

            CommandBuffer->Draw(3, 1, 0, 0, TexIndex);

            CommandBuffer->EndRenderPass();
        }

        RenderTargetSet.DepthScentil.RenderTarget = GTest->DepthTex;
        RenderTargetSet.DepthScentil.FinalLayout = EImageLayoutType::ShaderReadOnly;
        RenderTargetSet.DepthScentil.FinalUsage = EImageUsageType::ShaderResourceView;
        RenderTargetSet.DepthScentil.LoadOp = ERenderTargetLoadOp::Clear;
        RenderTargetSet.DepthScentil.StoreOp = ERenderTargetStoreOp::Store;
        RenderTargetSet.DepthScentil.ClearValue = CClearColor(Vec2{0.f, 0.f});
        
        {
            PROFILE_GPU_SCOPED_EVENT(CommandBuffer, "Geometry");

            CommandBuffer->BeginRenderPass(RenderPass);

            // Setup draw state
            {
                PROFILE_GPU_SCOPED_EVENT(CommandBuffer, "Draw Instanced Mesh");

                CommandBuffer->SetPipeline(GTest->Pipeline);

                CommandBuffer->SetIndexBuffer(GTest->IndexBuffer.Buffer, GTest->IndexBuffer.GetDeviceMemBufferByteOffset(), EFormatType::R16_UINT);
                CommandBuffer->SetVertexBuffer(GTest->VertexBuffer.Buffer, 0, GTest->VertexBuffer.GetDeviceMemBufferByteOffset());
                CommandBuffer->SetVertexBuffer(GTest->InstanceBuffer.Buffer, 1, GTest->InstanceBuffer.GetDeviceMemBufferByteOffset());

                CommandBuffer->Draw(6, 4, 0, 0, 0);
            }

            CommandBuffer->EndRenderPass();
        }
    }

    CommandBuffer->End();

    Result = CommandQueue->Submit(&CommandBuffer, 1);
    C_ASSERT_RETURN(C_SUCCEEDED(Result));

    ProcessedFrame = m_Renderer->GetCurrentFrame();
}

void CUnitTest::DeInit(IRenderer* Renderer)
{
    static bool IsDeInited = false;
    if (IsDeInited)
        return;

    IsDeInited = true;

    Vector<CHandle<CPipelineState>> PipelineStates = { GTest->Pipeline, GTest->FillTexPipeline, GTest->FillTex2Pipeline };
    for (auto& P : PipelineStates)
    {
        if (P.IsValid())
        {
            IResourceManager* ResourceManager = Renderer->GetRendererBackend()->GetResourceManager(CDeviceHandle::From(P));
            ResourceManager->ReleasePipelineStateCached(P);
        }
    }

    Vector<CHandle<CShader>> Shaders = {GTest->VertexShader, GTest->FullScreenQuadVertexShader,
        GTest->PixelShader, GTest->ProceduralPixelShader, GTest->CheckerboardPixelShader};
    for (auto& S : Shaders)
    {
        if (S.IsValid())
        {
            IResourceManager* ResourceManager = Renderer->GetRendererBackend()->GetResourceManager(CDeviceHandle::From(S));
            CASSERT(ResourceManager);
            ResourceManager->GetShader(S)->DeInit();
            ResourceManager->DestroyShader(S);
        }
    }

    if (GTest->LoadedTextSRV.IsValid())
    {
        CDeviceHandle DeviceHandle = CDeviceHandle::From(GTest->LoadedTextSRV);
        IBindlessManager* BindlessManager = Renderer->GetRendererBackend()->GetBindlessManager(DeviceHandle);
        CASSERT(BindlessManager);
        IResourceManager* ResourceManager = Renderer->GetRendererBackend()->GetResourceManager(DeviceHandle);
        CASSERT(ResourceManager);

        if (GTest->LoadedTexBindlessHandle != ~0u)
        {
            BindlessManager->UnRegisterResource(GTest->LoadedTexBindlessHandle);
            GTest->LoadedTexBindlessHandle = ~0u;
        }

        ResourceManager->DestroyResourceView(GTest->LoadedTextSRV);
        GTest->LoadedTextSRV = {};
    }

    if (GTest->LoadedTex.IsValid())
    {
        CDeviceHandle DeviceHandle = CDeviceHandle::From(GTest->LoadedTex);
        IResourceManager* ResourceManager = Renderer->GetRendererBackend()->GetResourceManager(DeviceHandle);
        CASSERT(ResourceManager);

        ResourceManager->DestroyResource(GTest->LoadedTex);
        GTest->LoadedTex = {};
    }

    CAllocHandle* Buffers[] = { &GTest->IndexBuffer, &GTest->VertexBuffer, &GTest->InstanceBindlessDataBuffer,
        &GTest->InstanceDataBuffer, &GTest->MaterialDataBuffer };

    for(auto& Buf : Buffers)
    {
        CDeviceHandle DeviceHandle = CDeviceHandle::From(Buf->Buffer);

        IBindlessManager* BindlessManager = Renderer->GetRendererBackend()->GetBindlessManager(DeviceHandle);
        CASSERT(BindlessManager);

        BindlessManager->FreePersistent(*Buf);
        *Buf = CAllocHandle{};
    }

    auto DeInitRenderTarget = [&](CRenderTarget& RT)
    {
        CHandle<CResourceView> Views[] = {
            RT.RenderTargetView, RT.DepthStencilView, RT.ShaderResourceView, RT.UnorderedAccessView,
        };
        for (auto& View : Views)
        {
            if (View.IsValid())
            {
                IResourceManager* ResourceManager = Renderer->GetRendererBackend()->GetResourceManager(CDeviceHandle::From(View));
                CASSERT(ResourceManager);

                ResourceManager->DestroyResourceView(View);
            }
        }

        if (RT.Texture.IsValid())
        {
            IResourceManager* ResourceManager = Renderer->GetRendererBackend()->GetResourceManager(CDeviceHandle::From(RT.Texture));
            CASSERT(ResourceManager);

            ResourceManager->GetResource(RT.Texture)->DeInit();
            ResourceManager->DestroyResource(RT.Texture);
        }
    };

    for (auto& T : GTest->Tex)
    {
        DeInitRenderTarget(T);
    }
    DeInitRenderTarget(GTest->DepthTex);
}

void GUnitTestDeInit(IRendererBackend* Backend)
{
    GTest->DeInit(Backend->GetRenderer());
    GTest.reset();
}


} // namespace Cyclone::Render

static std::vector<char> ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("failed to load file!");

    size_t filesize = static_cast<size_t>(file.tellg());

    std::vector<char> buffer(filesize);

    file.seekg(0);
    file.read(buffer.data(), buffer.size());

    file.close();

    return std::move(buffer);
}
