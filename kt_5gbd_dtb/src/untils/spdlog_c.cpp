#if 0
#include <string>
#include <iostream>
#include <cstdarg>  
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "spdlog_c.h"

// 定义一个全局日志器
static std::shared_ptr<spdlog::logger> g_rotating_logger;

// 初始化日志器
extern "C" int init_spdlog(const char* logger_name, int max_size, int max_files) 
{
    try {
        auto g_rotating_logger = spdlog::rotating_logger_mt("logger", logger_name, max_size, max_files);
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;

        return -1;
    }
    spdlog::set_default_logger(g_rotating_logger);
    spdlog::flush_on(spdlog::level::trace);

    return 0;
}

extern "C" void spdlog_info(const char* format, ...)
{
    if (g_rotating_logger) {
        va_list args;
        va_start(args, format);
        g_rotating_logger->info(format, args);
        va_end(args);
    }
}

extern "C" void spdlog_error(const char* format, ...)
{
    if (g_rotating_logger) {
        va_list args;
        va_start(args, format);
        g_rotating_logger->error(format, args);
        va_end(args);
    }
}

extern "C" void spdlog_warning(const char* format, ...)
{
    if (g_rotating_logger) {
        va_list args;
        va_start(args, format);
        g_rotating_logger->warn(format, args);
        va_end(args);
    }
}

extern "C" void spdlog_debug(const char* format, ...)
{
    if (g_rotating_logger) {
        va_list args;
        va_start(args, format);
        g_rotating_logger->debug(format, args);
        va_end(args);
    }
}

#else
#include <memory>
#include <cstdarg>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "spdlog_c.h"

// 定义一个全局日志器
static std::shared_ptr<spdlog::logger> g_rotating_logger = nullptr;

// 转换日志级别
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

static void log_formatted(log_level level, const char* fmt, va_list args) {
    char buffer[1024];  // 根据需求调整缓冲区大小
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
            auto g_rotating_logger = spdlog::rotating_logger_mt("rotating_logger", filename, max_size, max_files);
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

 // 快捷格式化方法
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
#endif