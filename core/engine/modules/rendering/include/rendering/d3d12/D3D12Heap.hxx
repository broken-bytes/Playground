#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <wrl.h>

#include "rendering/Heap.hxx"
#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include "rendering/d3d12/D3D12GPUResourceHandle.hxx"

namespace playground::rendering::d3d12 {
	class D3D12Heap : public Heap {
	public:
    	D3D12Heap(Microsoft::WRL::ComPtr<ID3D12Device> device,
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32_t numDescriptors,
			bool shaderVisible = false
		);

		[[nodiscard]]
		auto Size()->size_t;

		[[nodiscard]]
		auto Increment()->size_t;

		[[nodiscard]]
		auto HandleForHeapStart() -> std::shared_ptr<D3D12ResourceHandle>;

		[[nodiscard]]
		auto Native() -> Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>;

    	[[nodiscard]]
		auto HandleFor(std::uint32_t index, uint32_t id) -> std::shared_ptr<D3D12ResourceHandle>;

		[[nodiscard]]
		auto NextHandle() -> std::shared_ptr<D3D12ResourceHandle>;

		[[nodiscard]]
		auto IsFilled() -> bool;

		[[nodiscard]]
		auto CurrentIndex() const -> std::uint32_t;

	protected:
		uint32_t _index;
		bool _isShaderVisible;
		size_t _increment;
		D3D12_DESCRIPTOR_HEAP_DESC _desc;
		D3D12_DESCRIPTOR_HEAP_TYPE _type;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _heap;
		CD3DX12_CPU_DESCRIPTOR_HANDLE _heapStartCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE();
		CD3DX12_GPU_DESCRIPTOR_HANDLE _heapStartGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE();
		size_t _size;
	};
}
