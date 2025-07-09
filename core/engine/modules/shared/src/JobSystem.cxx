#include "shared/JobSystem.hxx"
#include "shared/Hardware.hxx"
#include <concurrentqueue.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif
#include <thread>
#include "shared/Arena.hxx"
#include <EASTL/fixed_vector.h>
#include <sstream>
#include <iostream>

namespace playground::jobsystem {
    using ArenaType = memory::VirtualArena;
    using Allocator = memory::ArenaAllocator<ArenaType>;
    ArenaType arena(128 * 1024 * 1024); // 128MB Physics Objects
    Allocator alloc(&arena, "Physics Allocator");

    struct CpuCore {
        uint32_t id;
        uint8_t efficiencyClass;
    };

    std::vector<CpuCore> GetCoresByEfficiency(bool wantEfficient) {
        DWORD len = 0;
        GetSystemCpuSetInformation(nullptr, 0, &len, GetCurrentProcess(), 0);

        std::vector<char> buffer(len);
        auto info = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(buffer.data());

        std::vector<CpuCore> cores;

        if (GetSystemCpuSetInformation(info, len, &len, GetCurrentProcess(), 0)) {
            char* ptr = buffer.data();
            while (ptr < buffer.data() + len) {
                auto* entry = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(ptr);

                if (entry->Type == CpuSetInformation) {
                    if (entry->CpuSet.Group == 0) {
                        bool isEfficient = entry->CpuSet.EfficiencyClass > 0;
                        if (isEfficient == wantEfficient) {
                            cores.push_back({
                                entry->CpuSet.LogicalProcessorIndex,
                                entry->CpuSet.EfficiencyClass
                                });
                        }
                    }
                }

                ptr += entry->Size;
            }
        }

        return cores;
    }

    void PinCurrentThreadToCore(uint32_t coreIndex) {
        GROUP_AFFINITY affinity = {};
        affinity.Group = 0;
        affinity.Mask = 1ull << coreIndex;

        if (!SetThreadGroupAffinity(GetCurrentThread(), &affinity, nullptr)) {
            exit(3);
        }
    }

    class Worker {
    public:
        Worker(std::string name, uint8_t index, bool isHighPerf, std::function<bool(std::shared_ptr<JobHandle>&)> pullJob) {
            _isRunning = true;
            _idleSpins = 0;
            auto cores = GetCoresByEfficiency(isHighPerf);
            
            _thread = std::thread([name, pullJob, cores, index, this]() {
#ifdef _WIN32
                std::wstring wStr;
                wStr.reserve(name.size() + 1);
                size_t convertedChars = 0;
                mbstowcs_s(&convertedChars, wStr.data(), name.size() + 1, name.data(), _TRUNCATE);
                SetThreadDescription(
                    GetCurrentThread(),
                    wStr.c_str()
                );
#endif
                if (!cores.empty()) {
                    uint32_t coreId = cores[index].id;
                    PinCurrentThreadToCore(coreId);
                }

                std::shared_ptr<JobHandle> nextJob;
                while (_isRunning) {
                    while (pullJob(nextJob)) {
                        nextJob->Add();
                        nextJob->Complete();
                        _idleSpins = 0;
                    }

                    if (_idleSpins < 100) {
                        ++_idleSpins;
                        std::this_thread::yield();
                    }
                    else {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
            });
        }

        void Stop() {
            _isRunning = false;
            _thread.join();
        }

    private:
        std::thread _thread;
        bool _isRunning;
        uint64_t _idleSpins;
    };

    JobHandle::~JobHandle() {
    }

    void JobHandle::Add() {
        _counter->fetch_add(1, std::memory_order_relaxed);
    }

    void JobHandle::Sub() {
        _counter->fetch_sub(1, std::memory_order_relaxed);
    }

    void JobHandle::Complete() {
        _work();

        _counter->fetch_sub(1, std::memory_order_release);

        auto parent = _parent.lock();

        if (parent == nullptr) {
            return;
        }

        parent->Sub();

        // When counter is 1 (only parent has a ref on its own left) we submit the parent
        if (parent->_counter->load() == 1) {
            Submit(parent);
        }
    }

    bool JobHandle::IsDone() const {
        return _counter->load(std::memory_order_acquire) == 0;
    }

    bool JobHandle::IsReady() const {
        return _counter->load(std::memory_order_acquire) == 1;
    }

    JobPriority JobHandle::Priority() const {
        return _priority;
    }

    void JobHandle::AddDependency(std::shared_ptr<JobHandle> dependency) {
        _dependencies.push_back(dependency);
        Add();
        dependency->_parent = shared_from_this();
    }

    JobHandle::JobHandle(std::string name, JobPriority priority, std::function<void()> work) : _name(name) {
        _priority = JobPriority::Low;
        _work = std::move(work);
        _counter = std::make_shared<std::atomic<uint64_t>>(1);
    }

    eastl::fixed_vector<std::shared_ptr<Worker>, 32, false, Allocator> workers(alloc);
    eastl::fixed_vector<std::shared_ptr<JobHandle>, 512, false, Allocator> pendingJobs(alloc);
    std::mutex pendingMutex;
    moodycamel::ConcurrentQueue<std::shared_ptr<JobHandle>> highPerfQueue;
    moodycamel::ConcurrentQueue<std::shared_ptr<JobHandle>> lowPerfQueue;

    uint8_t highPerfWorkers;
    uint8_t lowPerfWorkers;

    bool PullHighPerformanceTask(std::shared_ptr<JobHandle>& job) {
        if (highPerfQueue.try_dequeue(job)) {
            return true;
        }

        return false;
    }

    bool PullLowPerformanceTask(std::shared_ptr<JobHandle>& job) {
        if (lowPerfQueue.try_dequeue(job)) {
            return true;
        }

        return false;
    }

    void Init() {
        auto maxWorkers = hardware::CPUCount();

#if WIN32
        auto maxHighPerfWorkers = GetCoresByEfficiency(true).size();
        auto maxLowPerfWorkers = GetCoresByEfficiency(false).size();

        if (maxLowPerfWorkers == 0) {
            maxLowPerfWorkers = std::max(1, int(maxHighPerfWorkers * 0.25));
            maxHighPerfWorkers = std::max(1, int(maxHighPerfWorkers * 0.75));
        }
#endif

        if (maxHighPerfWorkers > 16) {
            maxHighPerfWorkers = 16;
        }

        if (maxLowPerfWorkers > 16) {
            maxLowPerfWorkers = 16;
        }

        highPerfWorkers = maxHighPerfWorkers;
        lowPerfWorkers = maxLowPerfWorkers;

        for (int x = 0; x < maxHighPerfWorkers; x++) {
            std::stringstream ss;
            ss << "H_WORKER_THREAD" << +x;

            workers.push_back(std::make_shared<Worker>(ss.str(), x, true, [](auto& job) { return PullHighPerformanceTask(job); }));
        }

        for (int x = 0; x < maxLowPerfWorkers; x++) {
            std::stringstream ss;
            ss << "E_WORKER_THREAD" << +x;

            workers.push_back(std::make_shared<Worker>(ss.str(), x, false, [](auto& job) { return PullLowPerformanceTask(job); }));
        }
    }

    void Submit(std::shared_ptr<JobHandle>& job) {
        auto it = std::find(pendingJobs.begin(), pendingJobs.end(), job);
        if (it != pendingJobs.end()) {
            pendingJobs.erase(it);
        }

        if (!job->IsReady()) {
            std::scoped_lock pendingLock{ pendingMutex };
            pendingJobs.push_back(job);
            return;
        }

        if (job->Priority() == JobPriority::High) {
            highPerfQueue.enqueue(job);
        }
        else {
            lowPerfQueue.enqueue(job);
        }
    }

    void Shutdown() {
        for (auto& worker : workers) {
            worker->Stop();
        }
    }

    uint8_t HighPerfWorkers() {
        return highPerfWorkers;
    }

    uint8_t LowPerfWorkers() {
        return lowPerfWorkers;
    }
}
