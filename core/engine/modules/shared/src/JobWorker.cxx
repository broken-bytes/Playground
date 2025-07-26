#include "shared/JobWorker.hxx"
#include "shared/JobHandle.hxx"
#include "shared/Logger.hxx"
#include <tracy/Tracy.hpp>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace playground::jobsystem {
    JobWorker::JobWorker(
        std::string name,
        uint8_t id,
        uint8_t coreIndex,
        hardware::CPUEfficiencyClass cpuEfficiency,
        std::mutex& mutex,
        std::condition_variable& conditionVar,
        std::function<bool(std::shared_ptr<JobHandle>&)> pullJob
    ) : _mutex(mutex), _conditionVar(conditionVar), _id(id) {
        _isRunning = true;
        _idleSpins = 0;
        auto cores = hardware::GetCoresByEfficiency(cpuEfficiency);

        _thread = std::thread([name, pullJob, cores, coreIndex, this]() {
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
                uint32_t coreId = cores[coreIndex].id;
                hardware::PinCurrentThreadToCore(coreId);
            }

            std::shared_ptr<JobHandle> nextJob;
            bool didRunJob = false;
            while (_isRunning) {
                if (!pullJob(nextJob)) {
                    ZoneScopedNC("Job System: Idle Wait", tracy::Color::Blue1);
                    std::unique_lock<std::mutex> lock(_mutex);
                    _conditionVar.wait(lock);
                    continue;
                } else {
                    ZoneScopedN("Job System: Execute Job");
                    ZoneText(name.c_str(), name.size());
                    ZoneText(nextJob->Name().c_str(), nextJob->Name().size());
                    ZoneColor(nextJob->TracerColour());
                    nextJob->Complete(_id);
                }
            }
        });
    }

    void JobWorker::Stop() {
        _isRunning = false;
    }

    void JobWorker::Join() {
        _thread.join();
    }
}
