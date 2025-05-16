#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>
#include <iostream>

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
    spdlog::set_default_logger(rotating_logger);
    spdlog::flush_on(spdlog::level::trace);

    return 0;
}

extern "C" void log_info(const char* message)
{
    if (g_rotating_logger) {
        g_rotating_logger->info(message);
    }
}

extern "C" void log_error(const char* message)
{
    if (g_rotating_logger) {
        g_rotating_logger->error(message);
    }
}

extern "C" void log_warning(const char* message)
{
    if (g_rotating_logger) {
        g_rotating_logger->warn(message);
    }
}

extern "C" void log_debug(const char* message)
{
    if (g_rotating_logger) {
        g_rotating_logger->debug(message);
    }
}

