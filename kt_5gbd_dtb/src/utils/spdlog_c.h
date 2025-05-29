#ifndef __SPDLOG_C_H
#define __SPDLOG_C_H

#ifdef __cplusplus
extern "C" {
#endif

int spdlog_c_init(const char* filename, int max_size,int max_files);

// 日志级别
typedef enum {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRITICAL
} log_level;

void spdlog_message(log_level level, const char* fmt, ...);
void spdlog_debug(const char* fmt, ...);
void spdlog_info(const char* fmt, ...);
void spdlog_error(const char* fmt, ...);

// // 清理资源
// void destroy_spdlogger(spdlogger logger);

#ifdef __cplusplus
}
#endif

#endif /* __SPDLOG_C_H */