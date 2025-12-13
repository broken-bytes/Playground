#pragma once

#include "RenderBackendType.hxx"
#include "d3d12/D3D12Device.hxx"

namespace playground::rendering {
	class DeviceFactory {
	public:
		static auto CreateDevice(RenderBackendType backend, uint8_t frameCount) -> std::shared_ptr<Device> {
			switch (backend)
			{
			case playground::rendering::RenderBackendType::Vulkan:
				break;
			case playground::rendering::RenderBackendType::D3D12:
				return std::make_shared<d3d12::D3D12Device>(frameCount);
				break;
			default:
				break;
			}

			// If we reach this point, the backend is not supported
			exit(1);
		}
	};
}
