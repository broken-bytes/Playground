#include "profiler/Profiler.hxx"
#include <tracy/Tracy.hpp>
#include <tracy/TracyD3D12.hpp>

#include <chrono>
#include <thread>

namespace playground::profiler {

    static std::chrono::steady_clock::time_point cpuStart;
    static std::chrono::steady_clock::time_point gpuStart;

    static CPUProfilerFrame cpuSamples[2];
    static GPUProfilerFrame gpuSamples[2];
    static int cpuIndex = 0;
    static int gpuIndex = 0;

    static TracyD3D12Ctx tracyD3D12Context = nullptr;

    void Init() {
#if ENABLE_PROFILER
        tracy::StartupProfiler();
#endif
    }

    void Shutdown() {
#if ENABLE_PROFILER
        tracy::ShutdownProfiler();
#endif
    }

    void RegisterThread(const char* name) {
        tracy::SetThreadName(name);
    }

    void StartCPUFrame() {
        FrameMarkStart("CPU");
    }

    void EndCPUFrame() {
        FrameMarkEnd("CPU");
    }

    ZoneHandle BeginZone(const char* name) {
        //auto zone = TracyCZoneN();

        return ZoneHandle{ };
    }

    void EndZone(ZoneHandle handle) {
        //tracy::EmitZoneEnd(handle.id);
    }

    void InitD3D12Context(ID3D12CommandQueue* queue, ID3D12Device* device) {
        if (!tracyD3D12Context) {
            //tracyD3D12Context = TracyD3D12Context(device, queue);
        }
    }

    void CollectGPUFrame(ID3D12GraphicsCommandList* list) {
        if (tracyD3D12Context) {
            //TracyD3D12Collect(tracyD3D12Context);
        }
    }

    void StartGPUFrame() {
        //FrameMarkStart("GPU");
    }

    void EndGPUFrame() {
        //FrameMarkEnd("GPU");
    }

    CPUProfilerFrame CPUStats() {
        return cpuSamples[(cpuIndex + 1) % 2];
    }

    GPUProfilerFrame GPUStats() {
        return gpuSamples[(gpuIndex + 1) % 2];
    }
}
