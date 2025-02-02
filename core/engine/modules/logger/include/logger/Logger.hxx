#pragma once

#include "logger/ILogger.hxx"
#include <memory>
#include <string_view>

namespace playground::logging::logger {
	auto SetLogger(std::shared_ptr<ILogger> logger) -> void;
	auto Info(std::string_view message) -> void;
	auto Warn(std::string_view message) -> void;
	auto Error(std::string_view message) -> void;
	auto SetLogLevel(LogLevel level) -> void;
}