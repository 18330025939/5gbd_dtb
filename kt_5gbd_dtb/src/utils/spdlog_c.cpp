#include <memory>
#include <cstdarg>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "spdlog_c.h"


/* 全局日志器 */
static std::shared_ptr<spdlog::logger> g_rotating_logger = nullptr;

static spdlog::level::level_enum convert_level(log_level level) {
    switch(level) {
        case LOG_TRACE:    return spdlog::level::trace;
        case LOG_DEBUG:    return spdlog::level::debug;
        case LOG_INFO:     return spdlog::level::info;
        case LOG_WARN:     return spdlog::level::warn;
        case LOG_ERROR:    return spdlog::level::err;
        case LOG_CRITICAL: return spdlog::level::critical;
        default:           return spdlog::level::info;
    }
}

static void log_formatted(log_level level, const char* fmt, va_list args) 
{
    char buffer[512];  
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    
    if (g_rotating_logger == nullptr) {
        spdlog::log(convert_level(level), buffer);
    } else {
        g_rotating_logger->log(convert_level(level), buffer);
    }
}

extern "C" {
    int spdlog_c_init(const char* filename, int max_size,int max_files) 
    {
        try {
            auto g_rotating_logger = spdlog::rotating_logger_mt("logger", filename, max_size, max_files);
            spdlog::set_level(spdlog::level::debug);
            spdlog::flush_on(spdlog::level::trace);

            spdlog::set_default_logger(g_rotating_logger);
            return 0;
        } catch (const spdlog::spdlog_ex& ex) {
            return -1;
        }
    }

    void spdlog_message(log_level level, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        log_formatted(level, fmt, args);
        va_end(args);
    }

    void spdlog_debug(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        log_formatted(LOG_DEBUG, fmt, args);
        va_end(args);
    }

    void spdlog_info(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        log_formatted(LOG_INFO, fmt, args);
        va_end(args);
    }

    void spdlog_error(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        log_formatted(LOG_ERROR, fmt, args);
        va_end(args);
    }
}


