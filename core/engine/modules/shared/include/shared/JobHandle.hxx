#pragma once

#include "shared/Job.hxx"
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace playground::jobsystem {
    class JobHandle : public std::enable_shared_from_this<JobHandle> {
    public:

        friend std::shared_ptr<JobHandle> Submit(Job job, std::function<void()> onCompletion);

        ~JobHandle();

        void Add();
        void Sub();
        void Set(uint16_t);
        void Complete(uint8_t workerId);
        bool IsDone() const;
        bool IsReady() const;
        JobPriority Priority() const;

        uint64_t Id() const {
            return _id;
        }

        std::string Name() const {
            return _name;
        }

        uint32_t TracerColour() const {
            return _tracerColour;
        }

        void Wait() const;

    private:
        JobHandle(std::string name, JobPriority priority, uint32_t tracerColour, std::function<void(uint8_t workerId)> work, std::function<void()> onCompletion);
        uint64_t _id;
        std::string _name;
        JobPriority _priority;
        uint32_t _tracerColour;
        std::shared_ptr<std::atomic<bool>> _isCompleted;
        std::shared_ptr<std::atomic<int64_t>> _references;
        std::function<void(uint8_t workerId)> _work;
        std::function<void()> _onCompletion;
    };
}
