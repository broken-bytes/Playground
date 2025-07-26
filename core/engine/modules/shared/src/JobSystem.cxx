#include "shared/Job.hxx"
#include "shared/JobHandle.hxx"
#include "shared/JobSystem.hxx"
#include "shared/JobWorker.hxx"
#include "shared/Hardware.hxx"
#include "shared/Logger.hxx"
#include <concurrentqueue.h>
#include <thread>
#include <tracy/Tracy.hpp>
#include "shared/Arena.hxx"
#include <EASTL/fixed_vector.h>
#include <EASTL/unordered_set.h>
#include <sstream>
#include <iostream>
#include <vector>

namespace playground::jobsystem {    
    std::vector<std::shared_ptr<JobWorker>> workers;
    std::vector<std::shared_ptr<JobHandle>> pendingJobs;
    std::mutex highPerfQueueMutex;
    std::mutex lowPerfQueueMutex;
    std::condition_variable conditionVarHghPerf;
    std::condition_variable conditionVarLowPerf;
    moodycamel::ConcurrentQueue<std::shared_ptr<JobHandle>> highPerfQueue;
    moodycamel::ConcurrentQueue<std::shared_ptr<JobHandle>> lowPerfQueue;

    uint8_t highPerfWorkers;
    uint8_t lowPerfWorkers;

    void SetupWorkers();
    bool PullHighPerformanceTask(std::shared_ptr<JobHandle>& job);
    bool PullLowPerformanceTask(std::shared_ptr<JobHandle>& job);
    void Push(std::shared_ptr<JobHandle> handle);

    void Init() {
        logging::logger::SetupSubsystem("jobs");
        SetupWorkers();
    }

    std::shared_ptr<JobHandle> Submit(Job job, std::function<void()> onCompletion) {
        ZoneScopedN("Job System Submit");

        std::shared_ptr<JobHandle> jobHandle = std::shared_ptr<JobHandle>(new JobHandle(job.Name, job.Priority, job.Color, job.Task, onCompletion));
        jobHandle->Set(job.Dependencies.size());

        if (!job.Dependencies.empty()) {
            for (auto& dep : job.Dependencies) {
                auto childHandle = Submit(dep, [jobHandle]() {
                    jobHandle->Sub();
                    if (jobHandle->IsReady()) {
                        Push(jobHandle);
                    }
                });
            }

            return jobHandle;
        }

        Push(jobHandle);

        return jobHandle;
    }

    std::shared_ptr<JobHandle> Submit(Job job) {
        return Submit(job, nullptr);
    }

    void Shutdown() {
        conditionVarHghPerf.notify_all();
        conditionVarLowPerf.notify_all();

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

    // ---- Helpers ----
    void SetupWorkers() {
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

            workers.push_back(std::make_shared<JobWorker>(
                ss.str(),
                x,
                highPerfCoresAvailable[coreCounter++],
                hardware::CPUEfficiencyClass::Performance,
                highPerfQueueMutex,
                conditionVarHghPerf,
                [](auto& job) { return PullHighPerformanceTask(job); })
            );
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

                workers.push_back(std::make_shared<JobWorker>(
                    ss.str(),
                    x,
                    highPerfCoresAvailable[coreCounter++],
                    hardware::CPUEfficiencyClass::Efficient,
                    lowPerfQueueMutex,
                    conditionVarLowPerf,
                    [](auto& job) { return PullLowPerformanceTask(job); })
                );
            }
        }
        else {
            for (int x = 0; x < maxLowPerfWorkers; x++) {
                std::stringstream ss;
                ss << "E_WORKER_THREAD" << +x;

                workers.push_back(std::make_shared<JobWorker>(
                    ss.str(),
                    x,
                    x,
                    hardware::CPUEfficiencyClass::Efficient,
                    lowPerfQueueMutex,
                    conditionVarLowPerf,
                    [](auto& job) { return PullLowPerformanceTask(job); })
                );
            }
        }
    }

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

    void Push(std::shared_ptr<JobHandle> handle) {
        if (handle->Priority() == JobPriority::High) {
            highPerfQueue.enqueue(handle);
            conditionVarHghPerf.notify_one();
        }
        else {
            lowPerfQueue.enqueue(handle);
            conditionVarLowPerf.notify_one();
        }
    }
}
