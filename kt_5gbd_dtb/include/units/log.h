#ifndef __SPDLOG_C_H
#define __SPDLOG_C_H

#ifdef __cplusplus
extern "C" {
#endif

int init_spdlog(const char* log_file_path, int max_size, int max_files);
void log_info(const char* message);
void log_error(const char* message);
void log_warning(const char* message);
void log_debug(const char* message);

#ifdef __cplusplus
}
#endif

#endif // __SPDLOG_C_H

