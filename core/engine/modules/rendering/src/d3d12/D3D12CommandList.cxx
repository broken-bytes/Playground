#include <sstream>
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include "rendering/d3d12/D3D12RenderTarget.hxx"

using namespace Microsoft::WRL;

namespace playground::rendering::d3d12 {
	D3D12CommandList::D3D12CommandList(
        const ComPtr<ID3D12Device9>& device,
        CommandListType type,
        uint8_t frameCount,
        std::string name
    ) : CommandList(type), _frameIndex(0) {
		// Create the command list
		D3D12_COMMAND_LIST_TYPE listType = D3D12_COMMAND_LIST_TYPE_DIRECT;

		switch (type)
		{
			case CommandListType::Graphics:
				listType = D3D12_COMMAND_LIST_TYPE_DIRECT;
				break;
			case CommandListType::Compute:
				listType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
				break;
			case CommandListType::Copy:
				listType = D3D12_COMMAND_LIST_TYPE_COPY;
				break;
			case CommandListType::Transfer:
				listType = D3D12_COMMAND_LIST_TYPE_COPY;
				break;
		}

        for (int x = 0; x < frameCount; x++) {
            ComPtr<ID3D12CommandAllocator> commandAllocator;

            if (FAILED(device->CreateCommandAllocator(listType, IID_PPV_ARGS(&commandAllocator))))
            {
                throw std::exception("Failed to create command allocator");
            }

            std::stringstream ss;
            ss << name << " Command Allocator " << x;

            auto name = ss.str();

            commandAllocator->SetName(std::wstring(name.begin(), name.end()).c_str());

            _commandAllocators.push_back(commandAllocator);
        }

		auto result = device->CreateCommandList(0, listType, _commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&_list));
		if (FAILED(result))
		{
			throw std::exception("Failed to create command list");
		}

        _list->SetName(std::wstring(name.begin(), name.end()).c_str());
    }

	auto D3D12CommandList::Native() -> ComPtr<ID3D12GraphicsCommandList> {
          return _list;
    }

	D3D12CommandList::~D3D12CommandList()
	{

	}

	auto D3D12CommandList::Begin() -> void
	{

	}

	auto D3D12CommandList::Close() -> void
	{
		_list->Close();
	}

    auto D3D12CommandList::Reset() -> void {
        _commandAllocators[_frameIndex]->Reset();
        _list->Reset(_commandAllocators[_frameIndex].Get(), nullptr);
        _frameIndex = (_frameIndex + 1) % _commandAllocators.size();
    }

	auto D3D12CommandList::ClearRenderTarget(std::shared_ptr<RenderTarget> handle, glm::vec4 color) -> void
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = std::static_pointer_cast<D3D12RenderTarget>(handle)->Handle();
		_list->ClearRenderTargetView(cpuHandle, reinterpret_cast<float*>(&color), 0, nullptr);
	}

	auto D3D12CommandList::SetViewport(
		uint32_t x,
		uint32_t y,
		uint32_t width,
		uint32_t height,
		uint32_t minDepth,
		uint32_t maxDepth
	) -> void
	{

	}

	auto D3D12CommandList::SetScissorRect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) -> void
	{

	}

	auto D3D12CommandList::SetPrimitiveTopology(PrimitiveTopology topology) -> void
	{

	}

	auto D3D12CommandList::SetMaterial(std::shared_ptr<Material>& material) -> void
	{

	}

	auto D3D12CommandList::BindVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer) -> void
	{

	}

	auto D3D12CommandList::BindIndexBuffer(std::shared_ptr<IndexBuffer>& vertexBuffer) -> void
	{

	}

	auto D3D12CommandList::DrawIndexed(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex) -> void
	{

	}
}
