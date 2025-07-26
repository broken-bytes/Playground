#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace playground::jobsystem {
    enum JobPriority {
        High,
        Low
    };

    struct Job {
        std::string Name;
        JobPriority Priority;
        uint64_t Color = 0; // Black tracy
        std::vector<Job> Dependencies;
        std::function<void(uint8_t workerId)> Task;
    };
}
