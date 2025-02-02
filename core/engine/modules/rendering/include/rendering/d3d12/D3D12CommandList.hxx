#pragma once

#include <directx/d3d12.h>
#include <wrl.h>
#include "rendering/CommandList.hxx"
#include "rendering/RootSignature.hxx"
#include "rendering/PipelineState.hxx"

namespace playground::rendering::d3d12 {
	class D3D12CommandList : public rendering::CommandList {
    public:
        D3D12CommandList(
            const Microsoft::WRL::ComPtr<ID3D12Device9>& device,
            CommandListType type,
            uint8_t frameCount,
            std::string name
        );
		auto Native() -> Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>;
		~D3D12CommandList() final;
		auto Begin() -> void override;
		auto Close() -> void override;
        auto Reset() -> void override;
        auto SetRenderTarget(std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void override;
        auto ClearDepthTarget(std::shared_ptr<DepthBuffer> target, float depth) -> void override;
		auto ClearRenderTarget(std::shared_ptr<RenderTarget> handle, glm::vec4 color) -> void override;
		auto SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t minDepth,
			uint32_t maxDepth) -> void override;
		auto SetScissorRect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) -> void override;
		auto SetPrimitiveTopology(PrimitiveTopology topology) -> void override;
        auto SetRootSignature(std::shared_ptr<RootSignature>& rootSignature) -> void override;
        auto SetPipelineState(std::shared_ptr<PipelineState>& pipelineState) -> void override;
		auto SetMaterial(std::shared_ptr<Material>& material) -> void override;
		auto BindVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer, uint8_t slot) -> void override;
		auto BindIndexBuffer(std::shared_ptr<IndexBuffer>& vertexBuffer) -> void override;
		auto DrawIndexed(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex) -> void override;
	private:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> _list;;
        std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> _commandAllocators;
        uint8_t _frameIndex;
        std::string _name;
	};
}
