#pragma once

#include <memory>
#include <vector>
#include <directx/d3d12.h>
#include <wrl.h>

#include "D3D12Device.hxx"
#include "rendering/CommandQueue.hxx"
#include "rendering/CommandList.hxx"
#include "rendering/Fence.hxx"

namespace playground::rendering::d3d12 {
    class D3D12CommandQueue : public rendering::CommandQueue {
        public:
        D3D12CommandQueue(
            Microsoft::WRL::ComPtr<ID3D12Device9> device,
            CommandListType type,
            std::string name
        );
        ~D3D12CommandQueue() final;
        auto Native() -> Microsoft::WRL::ComPtr<ID3D12CommandQueue>;

        void Execute(const std::vector<std::shared_ptr<rendering::CommandList>>&) override;

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _queue;
    };
}
