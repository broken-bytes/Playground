#pragma once

#include <dxgi1_6.h>
#include "rendering/TextureFormat.hxx"

namespace playground::rendering::d3d12
{
    auto DXGIFormatFrom(rendering::TextureFormat format)
    {
        switch (format)
        {
        case rendering::TextureFormat::Unknown:
            return DXGI_FORMAT_UNKNOWN;
        case rendering::TextureFormat::RGBA8:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case rendering::TextureFormat::RGBA16:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case rendering::TextureFormat::RGBA32:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case rendering::TextureFormat::RGB16:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case rendering::TextureFormat::RGB32:
            return DXGI_FORMAT_R32G32B32_UINT;
        case rendering::TextureFormat::BGRA8:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case rendering::TextureFormat::BGR8:
            return DXGI_FORMAT_B8G8R8X8_UNORM;
        }

        return DXGI_FORMAT_B8G8R8A8_UNORM;
    }
}
