#include "shared/JobWorker.hxx"
#include "shared/JobHandle.hxx"
#include <tracy/Tracy.hpp>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace playground::jobsystem {
    JobWorker::JobWorker(std::string name, uint8_t index, hardware::CPUEfficiencyClass cpuEfficiency, std::function<bool(std::shared_ptr<JobHandle>&)> pullJob) {
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
                    ZoneScopedN("Job System: Execute Job");
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

    void JobWorker::Stop() {
        _isRunning = false;
    }

    void JobWorker::Join() {
        _thread.join();
    }
}
