#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace playground::jobsystem {
    struct Job;
    class JobHandle;

    void Init();
    std::shared_ptr<JobHandle> Submit(Job job);
    void Shutdown();

    uint8_t HighPerfWorkers();
    uint8_t LowPerfWorkers();
}
