#pragma once

#include <string>
#include <string_view>

namespace playground::logging::logger {
    enum class LogLevel {
        Verbose,
        Debug,
        Warn,
        Error
    };

    void Init();
    void SetupSubsystem(std::string_view name);
    void SetLogLevel(LogLevel level);
    void Info_C(const char* message);
    void Debug_C(const char* message);
    void Warn_C(const char* message);
    void Error_C(const char* message);
	void Info(std::string_view message, std::string_view subsystem);
    void Debug(std::string_view message, std::string_view subsystem);
	void Warn(std::string_view message, std::string_view subsystem);
	void Error(std::string_view message, std::string_view subsystem);
}
