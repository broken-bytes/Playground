#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace playground::jobsystem {
    enum JobPriority {
        High,
        Low
    };

    class JobHandle : public std::enable_shared_from_this<JobHandle> {
    public:

        ~JobHandle();

        void Add();

        void Sub();

        void Complete();

        bool IsDone() const;

        bool IsReady() const;

        JobPriority Priority() const;

        void AddDependency(std::shared_ptr<JobHandle> dependency);

        static std::shared_ptr<JobHandle> Create(std::string name, JobPriority priority, uint32_t tracerColour, std::function<void()> work) {
            auto handle = std::shared_ptr<JobHandle>(new JobHandle(name, priority, tracerColour, std::move(work)));

            return handle;
        }

        static std::shared_ptr<JobHandle> Create(std::string name, JobPriority priority, std::function<void()> work) {
            auto handle = std::shared_ptr<JobHandle>(new JobHandle(name, priority, 0, std::move(work)));

            return handle;
        }

        std::string Name() const {
            return _name;
        }

        uint32_t TracerColour() const {
            return _tracerColour;
        }

    private:
        JobHandle(std::string name, JobPriority priority, uint32_t tracerColour, std::function<void()> work);

        std::string _name;
        JobPriority _priority;
        uint32_t _tracerColour;
        std::shared_ptr<std::atomic<uint64_t>> _counter;
        std::vector<std::shared_ptr<JobHandle>> _dependencies;
        std::weak_ptr<JobHandle> _parent;
        std::function<void()> _work;
    };
 
    void Init();
    void Submit(std::shared_ptr<JobHandle>& job);
    void Shutdown();

    uint8_t HighPerfWorkers();
    uint8_t LowPerfWorkers();
}
