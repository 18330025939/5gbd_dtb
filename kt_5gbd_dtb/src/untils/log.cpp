#include <string>
#include <iostream>
#include <cstdarg>  
#include <spdlog.h>
#include <sinks/stdout_color_sinks.h>
#include <sinks/rotating_file_sink.h>


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