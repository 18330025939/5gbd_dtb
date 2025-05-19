#if 0
#include <string>
#include <iostream>
#include <cstdarg>  
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

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

extern "C" void log_info(const char* format, ...)
{
    if (g_rotating_logger) {
        va_list args;
        va_start(args, format);
        g_rotating_logger->info(format, args);
        va_end(args);
    }
}

extern "C" void log_error(const char* format, ...)
{
    if (g_rotating_logger) {
        va_list args;
        va_start(args, format);
        g_rotating_logger->error(format, args);
        va_end(args);
    }
}

extern "C" void log_warning(const char* format, ...)
{
    if (g_rotating_logger) {
        va_list args;
        va_start(args, format);
        g_rotating_logger->warn(format, args);
        va_end(args);
    }
}

extern "C" void log_debug(const char* format, ...)
{
    if (g_rotating_logger) {
        va_list args;
        va_start(args, format);
        g_rotating_logger->debug(format, args);
        va_end(args);
    }
}

// spdlog_c.cpp
#include <cstdarg>
#include <cstdio>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog_c.h"

struct st_spdlog_logger {
    std::shared_ptr<spdlog::logger> logger;
} ;

namespace {
    std::string format_message(const char* fmt, va_list args)
    {
        va_list args_copy;
        va_copy(args_copy, args);
        int size = vsnprintf(nullptr, 0, fmt, args_copy);
        va_end(args_copy);

        if (size < 0) return "";
        
        std::vector<char> buf(size + 1);
        vsnprintf(buf.data(), buf.size(), fmt, args);
        return std::string(buf.data(), size);
    }
}
extern "C" {
    spdlog_logger* spdlog_init(const char* logger_name, const char* filename)
    {
        try {
            std::shared_ptr<spdlog::logger> logger;
            if (filename) {
                logger = spdlog::basic_logger_mt(logger_name, filename);
            } else {
                logger = spdlog::stdout_color_mt(logger_name);
            }
            logger->set_level(spdlog::level::trace);
            return new spdlog_logger{logger};
        } catch (const spdlog::spdlog_ex& ex) {
            fprintf(stderr, "spdlog init failed: %s\n", ex.what());
            return nullptr;
        }
    }

    void spdlog_shutdown(spdlog_logger* logger)
    {
        if (logger) {
            spdlog::drop(logger->logger->name());
            delete logger;
        }
    }
}
void spdlog_set_level(spdlog_logger* logger, spdlog_level level)
{
    if (logger && logger->logger) {
        logger->logger->set_level(static_cast<spdlog::level::level_enum>(level));
    }
}

void spdlog_log(spdlog_logger* logger, spdlog_level level, const char* fmt, ...)
{
    if (!logger || !logger->logger) return;

    va_list args;
    va_start(args, fmt);
    std::string msg = format_message(fmt, args);
    va_end(args);

    logger->logger->log(static_cast<spdlog::level::level_enum>(level), msg);
}
#endif
#include <memory>
#include <cstdarg>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "spdlog_c.h"


// 存储日志器的智能指针
static std::unordered_map<spdlogger, std::shared_ptr<spdlog::logger>> logger_map;
static size_t logger_counter = 0;

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

static void log_formatted(spdlogger logger, log_level level, const char* fmt, va_list args) {
    char buffer[1024];  // 根据需求调整缓冲区大小
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    
    if (logger == nullptr) {
        spdlog::log(convert_level(level), buffer);
    } else if (logger_map.find(logger) != logger_map.end()) {
        logger_map[logger]->log(convert_level(level), buffer);
    }
}

extern "C" {
    spdlogger spdlog_c_init(const char* filename, int max_size,int max_files) 
    {
        try {
            auto logger = spdlog::rotating_logger_mt("rotating_logger", filename, max_size, max_files);
            logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
            
            spdlogger handle = reinterpret_cast<spdlogger>(++logger_counter);
            logger_map[handle] = logger;
            return handle;
        } catch (const spdlog::spdlog_ex& ex) {
            return nullptr;
        }
    }

    void set_default_spdlogger(spdlogger logger) {
        if (logger_map.find(logger) != logger_map.end()) {
            spdlog::set_default_logger(logger_map[logger]);
        }
    }

    void spdlog_message(spdlogger logger, log_level level, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_formatted(logger, level, fmt, args);
        va_end(args);
    }

 // 快捷格式化方法
    void spdlog_debug(spdlogger logger, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_formatted(logger, LOG_DEBUG, fmt, args);
        va_end(args);
    }

    void spdlog_info(spdlogger logger, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_formatted(logger, LOG_INFO, fmt, args);
        va_end(args);
    }

    void spdlog_error(spdlogger logger, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_formatted(logger, LOG_ERROR, fmt, args);
        va_end(args);
    }

    void destroy_spdlogger(spdlogger logger) {
        if (logger_map.erase(logger) > 0) {
            spdlog::drop(logger_map[logger]->name());
        }
    }
    
}