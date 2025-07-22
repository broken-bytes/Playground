#include "shared/Logger.hxx"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <map>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace playground::logging::logger {
    struct Subsystem {
        std::string name;
        std::shared_ptr<spdlog::logger> logger;
    };

    LogLevel currentLogLevel = LogLevel::Verbose;
    std::map<std::string, Subsystem> subsystems;

    void Init() {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        AllocConsole();
#endif
    }

    void SetupSubsystem(std::string_view name) {
        auto subsystem = Subsystem{
            .name = std::string(name),
            .logger = spdlog::stdout_color_mt(std::string(name))
        };

        subsystems[std::string(name)] = subsystem;
        switch (currentLogLevel) {
            case LogLevel::Verbose:
                subsystem.logger->set_level(spdlog::level::trace);
                break;
            case LogLevel::Debug:
                subsystem.logger->set_level(spdlog::level::debug);
                break;
            case LogLevel::Warn:
                subsystem.logger->set_level(spdlog::level::warn);
                break;
            case LogLevel::Error:
                subsystem.logger->set_level(spdlog::level::err);
                break;
        }
    }

    void SetLogLevel(LogLevel level) {
        currentLogLevel = level;
        for (auto& [name, subsystem] : subsystems) {
            switch (level) {
                case LogLevel::Verbose:
                    subsystem.logger->set_level(spdlog::level::trace);
                    break;
                case LogLevel::Debug:
                    subsystem.logger->set_level(spdlog::level::debug);
                    break;
                case LogLevel::Warn:
                    subsystem.logger->set_level(spdlog::level::warn);
                    break;
                case LogLevel::Error:
                    subsystem.logger->set_level(spdlog::level::err);
                    break;
            }
        }
    }

    void Info_C(const char* message) {
        Info(std::string_view(message), "scripting");
    }

    void Warn_C(const char* message) {
        Warn(std::string_view(message), "scripting");
    }

    void Error_C(const char* message) {
        Error(std::string_view(message), "scripting");
    }

    void Info(std::string_view message, std::string_view subsystem) {
        subsystems[std::string(subsystem)].logger->info(message);
    }

    void Debug(std::string_view message, std::string_view subsystem) {
        subsystems[std::string(subsystem)].logger->debug(message);    
    }

    void Warn(std::string_view message, std::string_view subsystem) {
        subsystems[std::string(subsystem)].logger->warn(message);
    }

    void Error(std::string_view message, std::string_view subsystem) {
        subsystems[std::string(subsystem)].logger->error(message);
    }
}
