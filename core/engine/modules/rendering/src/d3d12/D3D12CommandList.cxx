#include <sstream>
#include "rendering/d3d12/D3D12Device.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"
#include "rendering/d3d12/D3D12CPUResourceHandle.hxx"
#include "rendering/d3d12/D3D12RenderTarget.hxx"
#include "rendering/d3d12/D3D12DepthBuffer.hxx"
#include "rendering/d3d12/D3D12IndexBuffer.hxx"
#include "rendering/d3d12/D3D12VertexBuffer.hxx"
#include "rendering/d3d12/D3D12ConstantBuffer.hxx"
#include "rendering/d3d12/D3D12Texture.hxx"
#include "rendering/d3d12/D3D12Sampler.hxx"
#include "rendering/d3d12/D3D12ReadbackBuffer.hxx"

using namespace Microsoft::WRL;

namespace playground::rendering::d3d12 {
	D3D12CommandList::D3D12CommandList(
        std::shared_ptr<D3D12Device> device,
        CommandListType type,
        std::string name
    ) : CommandList(type), _device(device) {
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

        ComPtr<ID3D12CommandAllocator> commandAllocator;

        if (FAILED(device->GetDevice()->CreateCommandAllocator(listType, IID_PPV_ARGS(&commandAllocator))))
        {
            throw std::exception("Failed to create command allocator");
        }

        std::stringstream ss;
        ss << name << "_ALLOCATOR";

        auto nameAlloc = ss.str();

        commandAllocator->SetName(std::wstring(nameAlloc.begin(), nameAlloc.end()).c_str());

        _commandAllocator = commandAllocator;
        

		auto result = device->GetDevice()->CreateCommandList(0, listType, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_list));
		if (FAILED(result))
		{
			throw std::exception("Failed to create command list");
		}

        _list->SetName(std::wstring(name.begin(), name.end()).c_str());
        _name = name;
    }

	auto D3D12CommandList::Native() -> ComPtr<ID3D12GraphicsCommandList7> {
          return _list;
    }

	D3D12CommandList::~D3D12CommandList()
	{
	}

	auto D3D12CommandList::Begin() -> void
	{
        if (_type == CommandListType::Graphics) {
            // Automatically bind the descriptor heaps
            auto heaps = _device->GetDescriptorHeaps();
            _list->SetDescriptorHeaps(heaps.size(), heaps.data());
        }
	}

	auto D3D12CommandList::Close() -> void
	{
		_list->Close();
	}

    auto D3D12CommandList::Reset() -> void {
        _commandAllocator->Reset();
        _list->Reset(_commandAllocator.Get(), nullptr);
    }

    auto D3D12CommandList::SetRenderTarget(std::shared_ptr<RenderTarget> colour, std::shared_ptr<DepthBuffer> depth) -> void {
        auto rtv = std::static_pointer_cast<D3D12RenderTarget>(colour)->Handle();
        auto dsv = std::static_pointer_cast<D3D12DepthBuffer>(depth)->Handle();
        _list->OMSetRenderTargets(
            1,
            &rtv,
            false,
            &dsv
        );
    }

    auto D3D12CommandList::ClearDepthTarget(std::shared_ptr<DepthBuffer> target, float depth) -> void {
        _list->ClearDepthStencilView(
            static_pointer_cast<D3D12DepthBuffer>(target)->Handle(),
            D3D12_CLEAR_FLAG_DEPTH,
            depth,
            0,
            0,
            nullptr
        );
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
        auto viewport = CD3DX12_VIEWPORT(x, y, width, height, minDepth, maxDepth);
        _list->RSSetViewports(1, &viewport);
	}

	auto D3D12CommandList::SetScissorRect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) -> void
	{
        auto rect = CD3DX12_RECT(left, top, right, bottom);
        _list->RSSetScissorRects(1, &rect);
	}

	auto D3D12CommandList::SetPrimitiveTopology(PrimitiveTopology topology) -> void
	{
        switch (topology)
        {
        case playground::rendering::PrimitiveTopology::POINT_LIST:
            _list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
            break;
        case playground::rendering::PrimitiveTopology::LINE_LIST:
            _list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
            break;
        case playground::rendering::PrimitiveTopology::LINE_STRIP:
            _list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);
            break;
        case playground::rendering::PrimitiveTopology::TRIANGLE_LIST:
            _list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            break;
        case playground::rendering::PrimitiveTopology::TRIANGLE_STRIP:
            _list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            break;
        default:
            break;
        }
	}

	auto D3D12CommandList::SetMaterial(std::shared_ptr<Material>& material) -> void
	{

	}

	auto D3D12CommandList::BindVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer, uint8_t slot) -> void
	{
        _list->IASetVertexBuffers(slot, 1, &std::static_pointer_cast<D3D12VertexBuffer>(vertexBuffer)->View());
	}

	auto D3D12CommandList::BindIndexBuffer(std::shared_ptr<IndexBuffer>& vertexBuffer) -> void
	{
        _list->IASetIndexBuffer(&std::static_pointer_cast<D3D12IndexBuffer>(vertexBuffer)->View());
	}

    auto D3D12CommandList::BindConstantBuffer(std::shared_ptr<ConstantBuffer> buffer, uint8_t slot) -> void {
        _list->SetGraphicsRootDescriptorTable(slot, std::static_pointer_cast<D3D12ConstantBuffer>(buffer)->GPUHandle());
    }

    auto D3D12CommandList::BindTexture(std::shared_ptr<Texture> texture, uint8_t slot) -> void {
        _list->SetGraphicsRootDescriptorTable(slot, std::static_pointer_cast<D3D12Texture>(texture)->GPUHandle());
    }

    auto D3D12CommandList::BindSampler(std::shared_ptr<Sampler> sampler, uint8_t slot) -> void {
        _list->SetGraphicsRootDescriptorTable(slot, std::static_pointer_cast<D3D12TextureSampler>(sampler)->GPUHandle());
    }

	auto D3D12CommandList::DrawIndexed(uint32_t numIndices, uint32_t startIndex, uint32_t startVertex) -> void
	{
        _list->DrawIndexedInstanced(numIndices, 1, startIndex, startVertex, 0);
	}
}
