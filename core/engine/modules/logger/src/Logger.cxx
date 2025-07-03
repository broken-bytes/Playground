#include "logger/Logger.hxx"
#include "logger/ILogger.hxx"
#include <mutex>
#include <vector>

namespace playground::logging::logger {
    std::vector<std::shared_ptr<ILogger>> loggers = {};
    std::mutex mutex;

	auto AddLogger(std::shared_ptr<ILogger> logger) -> void {
        playground::logging::logger::loggers.push_back(logger);
	}

	auto Info(const char* message) -> void {
        std::scoped_lock logLock(mutex);
        for (auto& logger : loggers) {
            logger->Info(message);
        }
	}

	auto Warn(const char* message) -> void {
        std::scoped_lock logLock(mutex);
        for (auto& logger : loggers) {
            logger->Warn(message);
        }
	}

	auto Error(const char* message) -> void {
        std::scoped_lock logLock(mutex);
        for (auto& logger : loggers) {
            logger->Error(message);
        }
	}

	auto SetLogLevel(LogLevel level) -> void {
        std::scoped_lock logLock(mutex);
        for (auto& logger : loggers) {
            logger->SetLogLevel(level);
        }
	}
}
