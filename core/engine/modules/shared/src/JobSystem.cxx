#include "shared/JobSystem.hxx"
#include "shared/Hardware.hxx"
#include <concurrentqueue.h>
#include <thread>
#include <tracy/Tracy.hpp>
#include "shared/Arena.hxx"
#include <EASTL/fixed_vector.h>
#include <sstream>
#include <iostream>

namespace playground::jobsystem {
    using ArenaType = memory::VirtualArena;
    using Allocator = memory::ArenaAllocator<ArenaType>;
    ArenaType arena(128 * 1024 * 1024); // 128MB Physics Objects
    Allocator alloc(&arena, "Physics Allocator");

    class Worker {
    public:
        Worker(std::string name, uint8_t index, hardware::CPUEfficiencyClass cpuEfficiency, std::function<bool(std::shared_ptr<JobHandle>&)> pullJob) {
            _isRunning = true;
            _idleSpins = 0;
            auto cores = hardware::GetCoresByEfficiency(cpuEfficiency);

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
                    hardware::PinCurrentThreadToCore(coreId);
                }

                std::shared_ptr<JobHandle> nextJob;
                while (_isRunning) {
                    while (pullJob(nextJob)) {
                        ZoneScopedN("Job System");
                        ZoneText(name.c_str(), name.size());
                        ZoneText(nextJob->Name().c_str(), nextJob->Name().size());
                        ZoneColor(nextJob->TracerColour());
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
        }

        void Join() {
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
        _counter->fetch_add(1, std::memory_order_release);
    }

    void JobHandle::Sub() {
        _counter->fetch_sub(1, std::memory_order_release);
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

    JobHandle::JobHandle(std::string name, JobPriority priority, uint32_t colour, std::function<void()> work) : _name(name) {
        _priority = priority;
        _work = work;
        _counter = std::make_shared<std::atomic<uint64_t>>(1);
        _tracerColour = colour;
    }

    eastl::vector<std::shared_ptr<Worker>, Allocator> workers(alloc);
    eastl::fixed_vector<std::shared_ptr<JobHandle>, 2048, false, Allocator> pendingJobs(alloc);
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
        int maxHighPerfWorkers = hardware::GetCoresByEfficiency(hardware::CPUEfficiencyClass::Performance).size() - 2;
        int maxLowPerfWorkers = hardware::GetCoresByEfficiency(hardware::CPUEfficiencyClass::Efficient).size();

        if (maxLowPerfWorkers == 0) {
            maxLowPerfWorkers = std::max(1, int(maxHighPerfWorkers * 0.25));
            maxHighPerfWorkers = std::max(2, int(maxHighPerfWorkers * 0.75));
        }
#endif

        highPerfWorkers = maxHighPerfWorkers;
        lowPerfWorkers = maxLowPerfWorkers;

        std::vector<uint64_t> highPerfCoresAvailable;
        // Reserve the first two cores for game and render thread
        auto highCores = hardware::GetCoresByEfficiency(hardware::CPUEfficiencyClass::Performance);
        for (int x = 2; x < highCores.size(); x++) {
            highPerfCoresAvailable.push_back(highCores[x].id);
        }

        int coreCounter = 0;
        for (int x = 0; x < maxHighPerfWorkers; x++) {
            std::stringstream ss;
            ss << "H_WORKER_THREAD" << +x;

            // If we run out of high performance cores, we loop back to the start (this is a fallback for when there are not enough cores, eg. on a Quadcore)
            if (coreCounter >= highPerfCoresAvailable.size()) {
                coreCounter = 0;
            }

            workers.push_back(std::make_shared<Worker>(ss.str(), highPerfCoresAvailable[coreCounter++], hardware::CPUEfficiencyClass::Performance, [](auto& job) { return PullHighPerformanceTask(job); }));
        }

        // If there are no low performance cores available, we use the high performance cores for low performance tasks as well
        if (hardware::GetCoresByEfficiency(hardware::CPUEfficiencyClass::Efficient).size() == 0) {
            int coreCounter = 0;
            for (int x = 0; x < maxHighPerfWorkers; x++) {
                std::stringstream ss;
                ss << "E_WORKER_THREAD" << +x;

                // If we run out of high performance cores, we loop back to the start (this is a fallback for when there are not enough cores, eg. on a Quadcore)
                if (coreCounter >= highPerfCoresAvailable.size()) {
                    coreCounter = 0;
                }

                workers.push_back(std::make_shared<Worker>(ss.str(), highPerfCoresAvailable[coreCounter++], hardware::CPUEfficiencyClass::Performance, [](auto& job) { return PullHighPerformanceTask(job); }));
            }
        }
        else {
            for (int x = 0; x < maxLowPerfWorkers; x++) {
                std::stringstream ss;
                ss << "E_WORKER_THREAD" << +x;

                workers.push_back(std::make_shared<Worker>(ss.str(), x, hardware::CPUEfficiencyClass::Efficient, [](auto& job) { return PullLowPerformanceTask(job); }));
            }
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

        for (auto& worker : workers) {
            worker->Join();
        }
    }

    uint8_t HighPerfWorkers() {
        return highPerfWorkers;
    }

    uint8_t LowPerfWorkers() {
        return lowPerfWorkers;
    }
}
