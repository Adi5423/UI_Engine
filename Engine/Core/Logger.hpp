#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

/**
 * Simple logging system for the engine
 * Professional implementation like Unity/Unreal/Godot logging system
 */
class Logger
{
public:
    enum class Level
    {
        INFO,
        WARNING,
        ERROR,
        DEBUG
    };

    static void Init(const std::string& filepath = "logs/Editor.log")
    {
        // Ensure directory exists
        std::filesystem::path path(filepath);
        if (path.has_parent_path())
        {
            std::filesystem::create_directories(path.parent_path());
        }

        Get().m_LogFile.open(filepath, std::ios::out | std::ios::app);
        if (Get().m_LogFile.is_open())
        {
            Log(Level::INFO, "--------------------------------------------------");
            Log(Level::INFO, "Logger initialized - Session Start");
        }
    }

    static void Shutdown()
    {
        if (Get().m_LogFile.is_open())
        {
            Log(Level::INFO, "Logger shutting down");
            Get().m_LogFile.close();
        }
    }

    static void Log(Level level, const std::string& message)
    {
        std::string timestamp = GetTimestamp();
        std::string levelStr = GetLevelString(level);
        
        std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
        
        // Console output
        std::cout << logMessage << std::endl;
        
        // File output
        if (Get().m_LogFile.is_open())
        {
            Get().m_LogFile << logMessage << std::endl;
            Get().m_LogFile.flush();
        }
    }

    static void Info(const std::string& message)
    {
        Log(Level::INFO, message);
    }

    static void Warning(const std::string& message)
    {
        Log(Level::WARNING, message);
    }

    static void Error(const std::string& message)
    {
        Log(Level::ERROR, message);
    }

    static void Debug(const std::string& message)
    {
        Log(Level::DEBUG, message);
    }

private:
    Logger() = default;
    ~Logger() = default;

    static Logger& Get()
    {
        static Logger instance;
        return instance;
    }

    static std::string GetTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    static std::string GetLevelString(Level level)
    {
        switch (level)
        {
            case Level::INFO:    return "INFO";
            case Level::WARNING: return "WARN";
            case Level::ERROR:   return "ERROR";
            case Level::DEBUG:   return "DEBUG";
            default:             return "UNKNOWN";
        }
    }

    std::ofstream m_LogFile;
};
