#ifndef __FIRMWARE_UPDATER_H
#define __FIRMWARE_UPDATER_H


#define SERVER_IP  "192.168.42.50"
#define SERVER_USERNAME "root"//"cktt"
#define SERVER_PASSWORD "root"//"cktt"

#define OTA_FILEUPLOAD_URL   "https://ota.cktt.com.cn/ota-server/fileUpload"

/* OTA升级报告路径格式： OTA_UPREPORT_PATH + task_id + / + filename*/
#define OTA_UPREPORT_REMOTE_PATH "/upgrade/"
#define OTA_UPREPORT_LOCAL_PATH "/upgrade/cktt/upgradereport/"

#define OTA_TASK_NOW  "AT_ONCE"
#define OTA_TASK_NEXT "NEXT_REBOOT"
#define OTA_TASK_REBOOTED "REBOOTED"

struct FileUploadfInfo
{   
    char dev_addr[10];
    char upload_path[80];
    char base64_str[1024*100];
} ;


struct UpdateRespInfo 
{
    char log_path[80];
    char time[64];
    char report[1024*2];
    char conf_path[80];
    // int conf_num;
    // struct ConfFileInfo *conf_info;
} ;

struct FwUpdateInfo
{
    char path[128];
    char name[64];
    char md5[64];
    uint16_t id;
    char type[20];
    char log_path[100];
    struct UpdateRespInfo resp_info;
} ;

struct FileUploadReport
{
    char dev_addr[10];
    char *file_path;
    char *base64_str;
} ;

#define UPGRADE_FILE_REMOTE_PATH "/upgrade"
/* 升级文件路径格式：UPGRADE_FILE_PATH + task_id + / + filename */
// #define UPGRADE_FILE_REMOTE_PATH "/upgrade/"
#define UPGRADE_FILE_LOCAL_PATH "/upgrade/cktt/"
#define MISSION_FILE_LOCAL_PATH "/upgrade/cktt/ota_mission"
#define UPTIME_FILE_LOCAL_PATH "/upgrade/cktt/uptime"

struct FwUpdater
{
    const char *dev_name;
    int (*trans_func)(void *);
    int (*update_func)(void *);
    int (*update_cb)(void *);
} ;

#define REGISTER_FIRMWARE_UPDATE_INTERFACE(name, trans, update, up_cb)\
    __attribute__((used, __section__("firmware_update"))) static struct FwUpdater firmware_update_##name = { \
        .dev_name = #name, \
        .trans_func = trans, \
        .update_func = update, \
        .update_cb = up_cb \
    }



#define UPDATE_LOG(file_name, string) \
    do { \
        FILE *fp = fopen(file_name, "a"); \
        if (fp != NULL) { \
            fprintf(fp, string); \
            fclose(fp); \
        } \
    } while (0)

#define UPDATE_LOG_FMT(file_name, fmt, ...) \
    do { \
        FILE *fp = fopen(file_name, "a"); \
        if (fp != NULL) { \
            fprintf(fp, fmt, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#endif /* __FIRMWARE_UPDATER_H */