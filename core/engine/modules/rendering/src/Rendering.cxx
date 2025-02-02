#include "rendering/Rendering.hxx"
#include "rendering/Device.hxx"
#include "rendering/Frame.hxx"
#include "rendering/VertexBuffer.hxx"
#include "rendering/DeviceFactory.hxx"
#include "rendering/Texture.hxx"
#include "rendering/renderpasses/OpaqueRenderPass.hxx"
#include "rendering/CommandQueue.hxx"
#include "rendering/GraphicsContext.hxx"
#include "rendering/RootSignature.hxx"
#include <map>
#include <memory>
#include <FreeImagePlus.h>

namespace playground::rendering {
	constexpr uint8_t FRAME_COUNT = 2;

	std::shared_ptr<Device> device = nullptr;
	void* window = nullptr;

	uint8_t frameIndex = 0;
	std::vector<std::shared_ptr<Frame>> frames = {};

	// Resource management
	std::map<uint64_t, std::shared_ptr<VertexBuffer>> vertexBuffers = {};
	std::map<uint64_t, std::shared_ptr<IndexBuffer>> indexBuffers = {};
	std::map<uint32_t, std::shared_ptr<Texture>> textures = {};
	std::map<uint32_t, std::shared_ptr<Shader>> shaders = {};
	std::map<uint32_t, std::shared_ptr<Material>> materials = {};
	std::map<uint32_t, std::shared_ptr<Mesh>> meshes = {};

	std::unique_ptr<GraphicsContext> graphicsContext = nullptr;

	std::shared_ptr<CommandList> opaqueCommandList = nullptr;
	std::shared_ptr<CommandList> transparentCommandList = nullptr;
	std::shared_ptr<CommandList> shadowCommandList = nullptr;
	std::shared_ptr<CommandList> uiCommandList = nullptr;
	std::shared_ptr<CommandList> transferCommandList = nullptr;

	std::unique_ptr<OpaqueRenderPass> opaqueRenderPass = nullptr;

    std::shared_ptr<VertexBuffer> vertexBuffer = nullptr;
    std::shared_ptr<IndexBuffer> indexBuffer = nullptr;

    std::shared_ptr<RootSignature> rootSignature = nullptr;

    std::shared_ptr<PipelineState> defaultPipelineState = nullptr;

    // Cube vertices
    Vertex cubeVertices[] =
    {
        { { -0.5f, -0.5f,  0.5f }, { 1, 0, 0, 1 }, { 0, 0, 1 }, { 0, 0 } },
        { {  0.5f, -0.5f,  0.5f }, { 0, 1, 0, 1 }, { 0, 0, 1 }, { 1, 0 } },
        { {  0.5f,  0.5f,  0.5f }, { 0, 0, 1, 1 }, { 0, 0, 1 }, { 1, 1 } },
        { { -0.5f,  0.5f,  0.5f }, { 1, 1, 0, 1 }, { 0, 0, 1 }, { 0, 1 } },

        { { -0.5f, -0.5f, -0.5f }, { 1, 0, 1, 1 }, { 0, 0, -1 }, { 0, 0 } },
        { {  0.5f, -0.5f, -0.5f }, { 0, 1, 1, 1 }, { 0, 0, -1 }, { 1, 0 } },
        { {  0.5f,  0.5f, -0.5f }, { 1, 1, 1, 1 }, { 0, 0, -1 }, { 1, 1 } },
        { { -0.5f,  0.5f, -0.5f }, { 0, 0, 0, 1 }, { 0, 0, -1 }, { 0, 1 } },
    };

    // Cube indices (2 triangles per face)
    uint32_t cubeIndices[] = {
        // Front face
        0, 1, 2,  2, 3, 0,

        // Back face
        5, 4, 7,  7, 6, 5,

        // Left face
        4, 0, 3,  3, 7, 4,

        // Right face
        1, 5, 6,  6, 2, 1,

        // Top face
        3, 2, 6,  6, 7, 3,

        // Bottom face
        4, 5, 1,  1, 0, 4
    };

    std::string vertexShaderCode = R"(
struct VSInput {
    float3 position : POSITION; // Vertex position in object space
    float4 color    : COLOR;    // Vertex color
    float3 normal   : NORMAL;   // Vertex normal
    float2 uv       : TEXCOORD; // UV coordinates
};

struct VSOutput {
    float4 position : SV_Position; // Transformed position in clip space
    float4 color    : COLOR;       // Pass-through color
    float3 normal   : NORMAL;      // Pass-through normal
    float2 uv       : TEXCOORD;    // Pass-through UV
};

VSOutput VSMain(VSInput input) {
    VSOutput output;

    // Construct the MVP matrix manually in the shader
    float4x4 modelMatrix = float4x4(
        1.0f, 0.0f, 0.0f, 0.0f, // Identity matrix (no transformation)
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

float4x4 viewMatrix = float4x4(
    1.0f, 0.0f, 0.0f, 0.0f,  // Right
    0.0f, 1.0f, 0.0f, 0.0f,  // Up
    0.0f, 0.0f, 1.0f, 0.0f,  // Forward
    0.0f, 0.0f, 5.0f, 1.0f   // Camera at Z = 5
);

float aspectRatio = 1280.0f / 720.0f; // Update as per your resolution
float fov = 1.0f / tan(3.14159265359f / 4.0f); // 45 degrees FOV
float nearPlane = 0.1f;
float farPlane = 100.0f;

float4x4 projectionMatrix = float4x4(
    fov / aspectRatio, 0.0f, 0.0f, 0.0f,
    0.0f, fov, 0.0f, 0.0f,
    0.0f, 0.0f, farPlane / (farPlane - nearPlane), 1.0f,
    0.0f, 0.0f, (-nearPlane * farPlane) / (farPlane - nearPlane), 0.0f
);


    // Combine the matrices
    float4x4 mvpMatrix = mul(modelMatrix, mul(viewMatrix, projectionMatrix));

    // Transform the vertex position
    output.position = mul(float4(input.position, 1.0f), mvpMatrix);

    // Pass through other attributes
    output.color = input.color;
    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}

        )";

    std::string pixelShaderCode = R"(
struct PSInput {
    float4 position : SV_Position;
    float4 color    : COLOR;
};

float4 PSMain(PSInput input) : SV_Target {
    return input.color; // Pass through the vertex color
}
        )";


	auto Init(void* window, uint32_t width, uint32_t height) -> void {
		FreeImage_Initialise();

		// Create a device
		device = DeviceFactory::CreateDevice(RenderBackendType::D3D12, window, FRAME_COUNT);

		// Create frames (frames hold all render things that need to alter between frames)
		for (int x = 0; x < FRAME_COUNT; x++)
		{
            std::stringstream ss;
            ss << "Frame " << x;

			auto rendertarget = device->CreateRenderTarget(width, height, TextureFormat::RGBA8, ss.str());

            ss.clear();
            ss << "DepthBuffer " << x;

			auto depthBuffer = device->CreateDepthBuffer(width, height, ss.str());

			frames.emplace_back(std::make_shared<Frame>(rendertarget, depthBuffer));
		}

		// Create all command lists
		opaqueCommandList = device->CreateCommandList(CommandListType::Graphics, "OpaqueCommandList");

		transparentCommandList = device->CreateCommandList(CommandListType::Graphics, "TransparentCommandList");

		shadowCommandList = device->CreateCommandList(CommandListType::Graphics, "ShadowCommandList");

		uiCommandList = device->CreateCommandList(CommandListType::Graphics, "UICommandList");

        transferCommandList = device->CreateCommandList(CommandListType::Transfer, "TransferCommandList");

		graphicsContext = device->CreateGraphicsContext(window, width, height, FRAME_COUNT);

        rootSignature = device->GetRootSignature();

        defaultPipelineState = device->CreatePipelineState(vertexShaderCode, pixelShaderCode);

		// Create the render passes
		opaqueRenderPass = std::make_unique<OpaqueRenderPass>();

        const UINT vertexBufferSize = sizeof(cubeVertices);

        vertexBuffer = device->CreateVertexBuffer(cubeVertices, sizeof(Vertex) * 8, sizeof(Vertex));

        indexBuffer = device->CreateIndexBuffer(cubeIndices, 36 * sizeof(uint32_t));
	}

	auto Shutdown() -> void {
        // Destroy all render passes first as these hold references to queues, buffers etc.
        opaqueRenderPass = nullptr;

        // Clear the contexts -> These hold swapchains and other resources
        graphicsContext = nullptr;

        // Close all command lists
        opaqueCommandList->Close();
        transparentCommandList->Close();
        shadowCommandList->Close();
        uiCommandList->Close();
        transferCommandList->Close();

        opaqueCommandList = nullptr;
        transparentCommandList = nullptr;
        shadowCommandList = nullptr;
        uiCommandList = nullptr;
        transferCommandList = nullptr;

        // Destroy all resources (render targets, depth buffers, etc)
        for (auto& frame : frames)
        {
            frame = nullptr;
        }

		// Cleanup
		device = nullptr;

        FreeImage_DeInitialise();
	}

	auto PreFrame() -> void {
		auto renderTarget = frames[frameIndex]->RenderTarget();
		auto depthBuffer = frames[frameIndex]->DepthBuffer();

        opaqueCommandList->SetRootSignature(rootSignature);
        opaqueCommandList->SetPipelineState(defaultPipelineState);
        opaqueCommandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLE_LIST);
        transparentCommandList->SetRootSignature(rootSignature);
        shadowCommandList->SetRootSignature(rootSignature);
        uiCommandList->SetRootSignature(rootSignature);

		opaqueRenderPass->Begin(opaqueCommandList, renderTarget, depthBuffer);
	}

	auto Update(float deltaTime) -> void {
		opaqueRenderPass->Execute();

        opaqueCommandList->BindVertexBuffer(vertexBuffer, 0);
        opaqueCommandList->BindIndexBuffer(indexBuffer);

        opaqueCommandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLE_LIST);

        opaqueCommandList->DrawIndexed(36, 0, 0);
	}

	auto PostFrame() -> void {
		opaqueRenderPass->End();

        opaqueCommandList->Close();
        transparentCommandList->Close();
        shadowCommandList->Close();
        uiCommandList->Close();
        transferCommandList->Close();

        graphicsContext->ExecuteCommandLists({ opaqueCommandList });
        graphicsContext->CopyToBackBuffer(frames[frameIndex]->RenderTarget());
		graphicsContext->Finish();

        opaqueCommandList->Reset();
        transparentCommandList->Reset();
        shadowCommandList->Reset();
        uiCommandList->Reset();

        transferCommandList->Reset();

        frameIndex = (frameIndex + 1) % FRAME_COUNT;
	}

	auto LoadShader(
		const std::string& code,
		ShaderType type
	) -> uint32_t {
		//const auto shader = device->CompileShader(code, type);
		//shaders[shader->id] = shader;

        return 0;
	}

	auto UnloadShader(uint64_t shader) -> void {
		device->DestroyShader(shader);
	}

	auto CreateVertexBuffer(const void* data, size_t size, size_t stride) -> VertexBufferHandle {
		auto buffer = device->CreateVertexBuffer(data, size, stride);
		auto id = buffer->Id();

		vertexBuffers[id] = buffer;

		return id;
	}

	auto CreateIndexBuffer(const uint32_t* indices, size_t len) -> IndexBufferHandle {
		auto buffer = device->CreateIndexBuffer(indices, len);
		auto id = buffer->Id();

		indexBuffers[id] = buffer;

		return id;
	}

	auto UpdateVertexBuffer(VertexBufferHandle buffer, const void* data, size_t size) -> void {

	}

	auto UpdateIndexBuffer(IndexBufferHandle buffer, const void* data, size_t size) -> void {

	}

	auto DrawIndexed(VertexBufferHandle vertexBuffer, IndexBufferHandle indexBuffer, MaterialHandle material) -> void {
	}
}
