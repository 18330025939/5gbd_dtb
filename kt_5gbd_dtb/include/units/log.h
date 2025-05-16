#ifndef __SPDLOG_C_H
#define __SPDLOG_C_H

#ifdef __cplusplus
extern "C" {
#endif

int init_spdlog(const char* log_file_path, int max_size, int max_files);
void log_info(const char* format, ...);
void log_error(const char* format, ...);
void log_warning(const char* format, ...);
void log_debugconst char* format, ...);

#ifdef __cplusplus
}
#endif

#endif // __SPDLOG_C_H

