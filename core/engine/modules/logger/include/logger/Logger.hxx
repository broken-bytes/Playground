#pragma once

#include "logger/ILogger.hxx"
#include <memory>
#include <string_view>

namespace playground::logging::logger {
	auto AddLogger(std::shared_ptr<ILogger> logger) -> void;
	auto Info(const char* message) -> void;
	auto Warn(const char* message) -> void;
	auto Error(const char* message) -> void;
	auto SetLogLevel(LogLevel level) -> void;
}
