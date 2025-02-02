#pragma once

#include <memory>
#include <directx/d3d12.h>
#include <wrl.h>

#include "rendering/CommandAllocator.hxx"
#include "rendering/CommandList.hxx"

namespace playground::rendering::d3d12
{
    class D3D12CommandAllocator : public rendering::CommandAllocator
    {
    public:
        D3D12CommandAllocator(
            const Microsoft::WRL::ComPtr<ID3D12Device>& device,
            CommandListType type,
            std::string name
        );
        auto Native() -> Microsoft::WRL::ComPtr<ID3D12CommandAllocator>;
        auto Reset() -> void override;
        auto ResetCommandList(std::shared_ptr<rendering::CommandList> list) -> std::shared_ptr<rendering::CommandList> override;

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
    };
}
