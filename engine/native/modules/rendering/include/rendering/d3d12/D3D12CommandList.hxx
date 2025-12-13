#pragma once

#include <directx/d3d12.h>
#include <wrl.h>
#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/CommandList.hxx"
#include <math/Vector4.hxx>

namespace playground::rendering::d3d12 {
	class D3D12CommandList : public rendering::CommandList {
    public:
        D3D12CommandList(
            std::shared_ptr<D3D12Device> device,
            CommandListType type,
            std::string name
        );
		auto Native() -> Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7>;
		~D3D12CommandList() final;
		auto Begin() -> void override;
		auto Close() -> void override;
        auto Reset() -> void override;
        auto SetRenderTarget(std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void override;
        auto ClearDepthTarget(std::shared_ptr<DepthBuffer> target, float depth) -> void override;
		auto ClearRenderTarget(std::shared_ptr<RenderTarget> handle, math::Vector4 color) -> void override;
		auto SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t minDepth,
			uint32_t maxDepth) -> void override;
		auto SetScissorRect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) -> void override;
		auto SetPrimitiveTopology(PrimitiveTopology topology) -> void override;
		auto SetMaterial(std::shared_ptr<Material>& material) -> void override;
		auto BindVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer, uint8_t slot) -> void override;
		auto BindIndexBuffer(std::shared_ptr<IndexBuffer>& vertexBuffer) -> void override;
		auto BindInstanceBuffer(std::shared_ptr<InstanceBuffer>& instanceBuffer) -> void override;
        auto BindDescriptorTable(std::shared_ptr<ConstantBuffer> buffer, uint8_t slot, uint32_t index) -> void override;
        auto BindConstantBuffer(std::shared_ptr<ConstantBuffer> buffer, uint8_t slot, uint32_t index) -> void override;
        auto BindTexture(std::shared_ptr<Texture> texture, uint8_t slot) -> void override;
        auto BindSampler(std::shared_ptr<Sampler> sampler, uint8_t slot) -> void override;
		auto DrawIndexed(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex, uint32_t numInstances, uint32_t startInstance) -> void override;
	private:
        std::shared_ptr<D3D12Device> _device;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> _list;;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
        std::string _name;
	};
}
