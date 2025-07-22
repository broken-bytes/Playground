#pragma once

#include "shared/Hardware.hxx"
#include <functional>
#include <string>
#include <thread>

namespace playground::jobsystem {
    class JobHandle;

    class JobWorker {
    public:
        JobWorker(std::string name, uint8_t index, hardware::CPUEfficiencyClass cpuEfficiency, std::function<bool(std::shared_ptr<JobHandle>&)> pullJob);

        void Stop();
        void Join();

    private:
        std::thread _thread;
        bool _isRunning;
        uint64_t _idleSpins;
    };
}
