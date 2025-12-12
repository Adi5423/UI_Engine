#include "Log.hpp"

#include <iostream>
#include <ctime>
#include <iomanip>
#include <filesystem>

namespace Core {

    std::shared_ptr<Log::Logger> Log::s_CoreLogger;
    std::shared_ptr<Log::Logger> Log::s_ClientLogger;
    std::ofstream Log::s_LogFile;

    void Log::Init()
    {
        // Create logs directory if it doesn't exist
        if (!std::filesystem::exists("logs"))
        {
            std::filesystem::create_directory("logs");
        }

        // Open/Create log file overwriting existing one
        s_LogFile.open("logs/engine.log", std::ios::out | std::ios::trunc);
        
        if (!s_LogFile.is_open())
        {
             std::cerr << "Failed to open log file logs/engine.log!" << std::endl;
        }

        s_CoreLogger = std::make_shared<Logger>("ENGINE");
        s_ClientLogger = std::make_shared<Logger>("APP");
    }

    void Log::Logger::Print(LogLevel level, const std::string& message)
    {
        // Get time
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        
        std::stringstream timeStr;
        timeStr << std::put_time(&tm, "%H:%M:%S");

        // -- Console Output (with colors) --
        const char* colorCode = "";
        const char* levelStr = "";
        
        switch (level)
        {
            case LogLevel::Trace: colorCode = "\033[90m"; levelStr = "TRACE"; break;
            case LogLevel::Info:  colorCode = "\033[32m"; levelStr = "INFO "; break;
            case LogLevel::Warn:  colorCode = "\033[33m"; levelStr = "WARN "; break;
            case LogLevel::Error: colorCode = "\033[31m"; levelStr = "ERROR"; break;
            case LogLevel::Fatal: colorCode = "\033[41m"; levelStr = "FATAL"; break;
        }

        const char* resetCode = "\033[0m";

        std::cout << colorCode 
                  << "[" << m_Name << "] " 
                  << "[" << timeStr.str() << "] "
                  << levelStr << ": " 
                  << message 
                  << resetCode << std::endl;

        // -- File Output (no colors) --
        if (Log::s_LogFile.is_open())
        {
            Log::s_LogFile << "[" << m_Name << "] " 
                           << "[" << timeStr.str() << "] "
                           << levelStr << ": " 
                           << message 
                           << std::endl;
            // Optionally flush if critical? 
            if (level >= LogLevel::Error) Log::s_LogFile.flush();
        }
    }
}
