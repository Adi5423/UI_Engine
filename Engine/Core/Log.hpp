#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <fstream> // For file logging
#include <sstream>

namespace Core {
    
    // Log levels
    enum class LogLevel {
        Trace = 0,
        Info,
        Warn,
        Error,
        Fatal
    };

    class Log {
    public:
        static void Init();

        // Core log functions
        template<typename... Args>
        static void CoreTrace(Args&&... args) { GetCoreLogger()->Log(LogLevel::Trace, std::forward<Args>(args)...); }

        template<typename... Args>
        static void CoreInfo(Args&&... args) { GetCoreLogger()->Log(LogLevel::Info, std::forward<Args>(args)...); }

        template<typename... Args>
        static void CoreWarn(Args&&... args) { GetCoreLogger()->Log(LogLevel::Warn, std::forward<Args>(args)...); }

        template<typename... Args>
        static void CoreError(Args&&... args) { GetCoreLogger()->Log(LogLevel::Error, std::forward<Args>(args)...); }

        // Client log functions
        template<typename... Args>
        static void Trace(Args&&... args) { GetClientLogger()->Log(LogLevel::Trace, std::forward<Args>(args)...); }

        template<typename... Args>
        static void Info(Args&&... args) { GetClientLogger()->Log(LogLevel::Info, std::forward<Args>(args)...); }

        template<typename... Args>
        static void Warn(Args&&... args) { GetClientLogger()->Log(LogLevel::Warn, std::forward<Args>(args)...); }

        template<typename... Args>
        static void Error(Args&&... args) { GetClientLogger()->Log(LogLevel::Error, std::forward<Args>(args)...); }

    private:
        // Internal logger class
        class Logger {
        public:
            Logger(const std::string& name) : m_Name(name) {}

            template<typename... Args>
            void Log(LogLevel level, Args&&... args) {
                std::stringstream ss;
                (ss << ... << args);
                Print(level, ss.str());
            }

        private:
            void Print(LogLevel level, const std::string& message);
            std::string m_Name;
        };

        static std::shared_ptr<Logger> s_CoreLogger;
        static std::shared_ptr<Logger> s_ClientLogger;
        static std::ofstream s_LogFile; // File sink

        static std::shared_ptr<Logger>& GetCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<Logger>& GetClientLogger() { return s_ClientLogger; }
    };
}

// Core log macros
#define CORE_TRACE(...)    ::Core::Log::CoreTrace(__VA_ARGS__)
#define CORE_INFO(...)     ::Core::Log::CoreInfo(__VA_ARGS__)
#define CORE_WARN(...)     ::Core::Log::CoreWarn(__VA_ARGS__)
#define CORE_ERROR(...)    ::Core::Log::CoreError(__VA_ARGS__)
#define CORE_FATAL(...)    ::Core::Log::CoreError("FATAL: ", __VA_ARGS__)

// Client log macros
#define LOG_TRACE(...)     ::Core::Log::Trace(__VA_ARGS__)
#define LOG_INFO(...)      ::Core::Log::Info(__VA_ARGS__)
#define LOG_WARN(...)      ::Core::Log::Warn(__VA_ARGS__)
#define LOG_ERROR(...)     ::Core::Log::Error(__VA_ARGS__)
#define LOG_FATAL(...)     ::Core::Log::Error("FATAL: ", __VA_ARGS__)
