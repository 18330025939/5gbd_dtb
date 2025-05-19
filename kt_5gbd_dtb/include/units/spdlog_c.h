#ifndef SPDLOG_C_H
#define SPDLOG_C_H
#if 0
#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_spdlog_logger spdlog_logger;

typedef enum {
    SPDLOG_C_LEVEL_TRACE = 0,
    SPDLOG_C_LEVEL_DEBUG,
    SPDLOG_C_LEVEL_INFO,
    SPDLOG_C_LEVEL_WARN,
    SPDLOG_C_LEVEL_ERROR,
    SPDLOG_C_LEVEL_CRITICAL,
    SPDLOG_C_LEVEL_OFF
} spdlog_level;

spdlog_logger* spdlog_init(const char* logger_name, const char* filename);
void spdlog_shutdown(spdlog_logger* logger);

void spdlog_set_level(spdlog_logger* logger, spdlog_level level);

void spdlog_log(spdlog_logger* logger, spdlog_level level, const char* fmt, ...);

#define spdlog_trace(logger, ...) spdlog_log(logger, SPDLOG_C_LEVEL_TRACE, __VA_ARGS__)
#define spdlog_debug(logger, ...) spdlog_log(logger, SPDLOG_C_LEVEL_DEBUG, __VA_ARGS__)
#define spdlog_info(logger, ...)  spdlog_log(logger, SPDLOG_C_LEVEL_INFO, __VA_ARGS__)
#define spdlog_warn(logger, ...)  spdlog_log(logger, SPDLOG_C_LEVEL_WARN, __VA_ARGS__)
#define spdlog_error(logger, ...) spdlog_log(logger, SPDLOG_C_LEVEL_ERROR, __VA_ARGS__)
#define spdlog_critical(logger, ...) spdlog_log(logger, SPDLOG_C_LEVEL_CRITICAL, __VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif
#ifdef __cplusplus
extern "C" {
#endif
#if 1
// 日志器句柄类型（不透明指针）
typedef void* spdlogger;

// 初始化旋转日志器 (max_size单位: MB)
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

// 清理资源
void destroy_spdlogger(spdlogger logger);
#else
int init_spdlog(const char* logger_name, int max_size, int max_files);
void spdlog_info(const char* format, ...);
void spdlog_error(const char* format, ...);
void spdlog_warning(const char* format, ...);
void spdlog_debug(const char* format, ...);
#endif
#ifdef __cplusplus
}
#endif

#endif // SPDLOG_C_H