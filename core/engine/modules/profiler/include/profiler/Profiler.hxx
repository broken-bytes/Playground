#pragma once

#include <string>
#include <cstdint>
#include <directx/d3d12.h>

namespace playground::profiler {
    struct ZoneHandle {
        uint64_t zoneId;
        const void* src;
    };

    void Init();       // Call once from engine
    void Shutdown();   // Call once at end

    // Thread
    void RegisterThread(const char* name);

    // CPU
    void StartCPUFrame();
    void EndCPUFrame();

    ZoneHandle BeginZone(const char* name);

    void EndZone(ZoneHandle handle);

    // GPU
    void InitD3D12Context(ID3D12CommandQueue* queue, ID3D12Device* device);
    void CollectGPUFrame(ID3D12GraphicsCommandList* list);
    void StartGPUFrame();
    void EndGPUFrame();

    // Optional stats struct
    struct CPUProfilerFrame {
        int64_t cpuTimeNs = 0;
    };

    struct GPUProfilerFrame {
        int64_t gpuTimeNs = 0;
    };

    CPUProfilerFrame CPUStats();
    GPUProfilerFrame GPUStats();
}
