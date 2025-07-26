#pragma once

#include "shared/Hardware.hxx"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace playground::jobsystem {
    class JobHandle;

    class JobWorker {
    public:
        JobWorker(
            std::string name,
            uint8_t index,
            uint8_t coreIndex,
            hardware::CPUEfficiencyClass cpuEfficiency,
            std::mutex& mutex,
            std::condition_variable& conditionVar,
            std::function<bool(std::shared_ptr<JobHandle>&)> pullJob
        );

        void Stop();
        void Join();

    private:
        uint8_t _id = 0;
        std::thread _thread;
        std::mutex& _mutex;
        std::condition_variable& _conditionVar;
        bool _isRunning;
        uint64_t _idleSpins;
    };
}
