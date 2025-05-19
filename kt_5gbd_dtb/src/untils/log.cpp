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
#endif 
// spdlog_c.cpp
#include <cstdarg>
#include <cstdio>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog_c.h"

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

spdlog_logger_t* spdlog_init(const char* logger_name, const char* filename)
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