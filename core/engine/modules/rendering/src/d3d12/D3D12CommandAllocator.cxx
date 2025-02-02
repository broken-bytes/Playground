#include "rendering/d3d12/D3D12CommandAllocator.hxx"

#include <exception>

#include "rendering/d3d12/D3D12CommandList.hxx"

namespace playground::rendering::d3d12
{
    D3D12CommandAllocator::D3D12CommandAllocator(
        const Microsoft::WRL::ComPtr<ID3D12Device>& device,
        CommandListType type,
        std::string name
    )
    {
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

        if (FAILED(device->CreateCommandAllocator(listType, IID_PPV_ARGS(&_commandAllocator))))
        {
            throw std::exception("Failed to create command allocator");
        }

        _commandAllocator->SetName(std::wstring(name.begin(), name.end()).c_str());
    }

    auto D3D12CommandAllocator::Native() -> Microsoft::WRL::ComPtr<ID3D12CommandAllocator>
    {
        return _commandAllocator;
    }


    auto D3D12CommandAllocator::Reset() -> void
    {
        if (FAILED(_commandAllocator->Reset())) {
            throw std::exception("Failed to reset command allocator");
        }
    }

    auto D3D12CommandAllocator::ResetCommandList(std::shared_ptr<rendering::CommandList> list) -> std::shared_ptr<rendering::CommandList>
    {
        if (FAILED(std::static_pointer_cast<D3D12CommandList>(list)->Native()->Reset(_commandAllocator.Get(), nullptr))) {
            throw std::exception("Failed to reset command list");
        }

        return list;
    }
}
