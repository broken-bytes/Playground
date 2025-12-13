#include "shared/JobHandle.hxx"
#include <thread>
#include <iostream>
#include <cassert>
#include "shared/Logger.hxx"
#include <tracy/Tracy.hpp>

namespace playground::jobsystem {
    std::atomic<uint64_t> nextJobId = 0;

    JobHandle::~JobHandle() {
    }

    void JobHandle::Add() {
        _references->fetch_add(1, std::memory_order_acquire);
    }

    void JobHandle::Sub() {
        _references->fetch_sub(1, std::memory_order_acquire);
    }

    void JobHandle::Set(uint16_t references) {
        _references->store(references, std::memory_order_relaxed);
    }

    void JobHandle::Complete(uint8_t workerId) {
        ZoneScopedNC("Job System: Complete Job", tracy::Color::PaleVioletRed1);
        if (_work) {
            ZoneScopedNC("Job System: Execute Work", tracy::Color::PaleVioletRed3);
            ZoneText(Name().c_str(), Name().length());
            _work(workerId);
        }

        _isCompleted->store(true, std::memory_order_release);

        if (_onCompletion) {
            ZoneScopedNC("Job System: On Completion", tracy::Color::PaleVioletRed4);
            _onCompletion();
        }
    }

    bool JobHandle::IsDone() const {
        return _isCompleted->load(std::memory_order_acquire);
    }

    void JobHandle::Wait() const {
        ZoneScopedNC("Job System: Wait for Job", tracy::Color::PaleVioletRed2);
        while (!IsDone()) {
            std::this_thread::yield();
        }
    }

    bool JobHandle::IsReady() const {
        return _references->load(std::memory_order_acquire) == 0;
    }

    JobPriority JobHandle::Priority() const {
        return _priority;
    }

    JobHandle::JobHandle(std::string name, JobPriority priority, uint32_t colour, std::function<void(uint8_t workerId)> work, std::function<void()> onCompletion) : _name(name) {
        _id = nextJobId.fetch_add(1, std::memory_order_acquire);
        _priority = priority;
        _work = work;
        _onCompletion = onCompletion;
        _isCompleted = std::make_shared<std::atomic<bool>>(false);
        _references = std::make_shared<std::atomic<int64_t>>(0);
        _tracerColour = colour;
    }
}
