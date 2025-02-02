#include "rendering/d3d12/D3D12CommandQueue.hxx"
#include "rendering/d3d12/D3D12CommandList.hxx"

using namespace playground::rendering;
using namespace Microsoft::WRL;

namespace playground::rendering::d3d12 {
    D3D12CommandQueue::D3D12CommandQueue(
        ComPtr<ID3D12Device9> device,
        CommandListType type,
        std::string name
    ) : CommandQueue(type)
    {
        
    }

    D3D12CommandQueue::~D3D12CommandQueue()
    {
        _queue->Release();
    }

    auto D3D12CommandQueue::Native() -> Microsoft::WRL::ComPtr<ID3D12CommandQueue>
    {
        return _queue;
    }

    void D3D12CommandQueue::Execute(const std::vector<std::shared_ptr<rendering::CommandList>>& lists)
    {
        auto listsToExecute = std::vector<ID3D12CommandList*>();

        for (auto& list: lists)
        {
            listsToExecute.emplace_back(reinterpret_cast<D3D12CommandList*>(list.get())->Native().Get());
        }
    }

    void D3D12CommandQueue::Signal(rendering::Fence& fence, uint64_t value)
    {

    }
}
