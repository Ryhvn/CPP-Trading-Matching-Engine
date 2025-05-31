#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <atomic>

namespace me {
    enum class LogLevel { INFO, WARN, ERROR };

    // Flag global pour activer/désactiver les logs
    extern std::atomic<bool> g_loggingEnabled;

    inline const char* toString(LogLevel l) {
        switch (l) {
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
        }
        return "UNK";
    }

    inline void log(LogLevel lvl, const std::string& msg) {
        if (!g_loggingEnabled.load(std::memory_order_relaxed)) return;
        auto now = std::chrono::system_clock::now();
        auto tt  = std::chrono::system_clock::to_time_t(now);
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch()).count() % 1000;
        std::cout
          << std::put_time(std::localtime(&tt), "%F %T")
          << '.' << std::setfill('0') << std::setw(3) << ms
          << ' ' << toString(lvl)
          << ' ' << msg << "\n";
    }

#define LOG_INFO(msg)  ::me::log(::me::LogLevel::INFO,  msg)
#define LOG_WARN(msg)  ::me::log(::me::LogLevel::WARN,  msg)
#define LOG_ERROR(msg) ::me::log(::me::LogLevel::ERROR, msg)

    // Pour piloter la flag runtime
    inline void setLoggingEnabled(bool e) {
        g_loggingEnabled.store(e, std::memory_order_relaxed);
    }
}
