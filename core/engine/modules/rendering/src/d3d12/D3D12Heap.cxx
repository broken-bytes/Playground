#include "rendering/d3d12/D3D12Heap.hxx"
#include <cstdint>
#include <stdexcept>
#include <memory>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <wrl.h>

namespace playground::rendering::d3d12
{
	D3D12Heap::D3D12Heap(
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32_t numDescriptors,
		bool shaderVisible
	) : _index(0), _isShaderVisible(shaderVisible) {
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = numDescriptors;
		desc.Type = type;
		desc.Flags = (shaderVisible) ?
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE :
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (S_OK == device->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(&_heap))
			) {
			_desc = desc;
			_increment = device->GetDescriptorHandleIncrementSize(type);
			_type = type;
			_isShaderVisible = shaderVisible;
			_heapStartCPU = _heap->GetCPUDescriptorHandleForHeapStart();
			if (shaderVisible) {
				_heapStartGPU = _heap->GetGPUDescriptorHandleForHeapStart();
			}
		}
		else {
			throw std::runtime_error("Failed to create Heap");
		}
	}

	auto D3D12Heap::Size()->size_t {
		return _size;
	};

	auto D3D12Heap::Increment()->size_t {
		return _increment;
	};

	auto D3D12Heap::HandleForHeapStart() -> std::shared_ptr<D3D12ResourceHandle> {
		auto cdx = CD3DX12_CPU_DESCRIPTOR_HANDLE(_heap->GetCPUDescriptorHandleForHeapStart());
        if (!_isShaderVisible) {
            return std::make_shared<D3D12ResourceHandle>(cdx);
        }
        auto gdx = CD3DX12_GPU_DESCRIPTOR_HANDLE(_heap->GetGPUDescriptorHandleForHeapStart());
		return std::make_shared<D3D12ResourceHandle>(cdx, gdx);
	};

	auto D3D12Heap::Native() -> Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> {
		return _heap;
	}

	auto D3D12Heap::HandleFor(std::uint32_t index) -> std::shared_ptr<D3D12ResourceHandle> {
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(_heap->GetCPUDescriptorHandleForHeapStart(), index, _increment);
        if (!_isShaderVisible) {
            return std::make_shared<D3D12ResourceHandle>(handle);
        }

        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(_heap->GetGPUDescriptorHandleForHeapStart(), index, _increment);

		return std::make_shared<D3D12ResourceHandle>(handle, gpuHandle);
	}

	auto D3D12Heap::NextHandle() -> std::shared_ptr<D3D12ResourceHandle>
	{
		assert(_index < _desc.NumDescriptors);
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(_heap->GetCPUDescriptorHandleForHeapStart(), _index, _increment);
        if (!_isShaderVisible) {
            _index++;
            return std::make_shared<D3D12ResourceHandle>(handle);
        }
        CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(_heap->GetGPUDescriptorHandleForHeapStart(), _index, _increment);

        _index++;
		return std::make_shared<D3D12ResourceHandle>(handle, gpuHandle);
	}

	auto D3D12Heap::IsFilled() -> bool
	{
		return _index >= _desc.NumDescriptors;
	}

	auto D3D12Heap::CurrentIndex() const -> std::uint32_t
	{
		return _index;
	}
}
