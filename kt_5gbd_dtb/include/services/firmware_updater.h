#ifndef __FIRMWARE_UPDATER_H
#define __FIRMWARE_UPDATER_H

// #define BUFFER_SIZE 4096    // 根据实际数据流量调整
// #define SYNC1 0x50
// #define SYNC2 0x42

// // 全局循环缓冲区
// typedef struct {
//     uint8_t data[BUFFER_SIZE];
//     size_t head;    // 写指针
//     size_t tail;    // 读指针
// } CircularBuffer;

// CircularBuffer rx_buffer;
// pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;


#define SERVER_IP  "192.168.42.50"
#define SERVER_USERNAME "cktt"
#define SERVER_PASSWORD "cktt"

/* OTA升级报告路径格式： OTA_UPREPORT_PATH + task_id + / + filename*/
#define OTA_UPREPORT_REMOTE_PATH "/upgrade/"
#define OTA_UPREPORT_LOCAL_PATH "/upgrade/cktt/upgradereport/"

struct FwUpdateInfo
{
    char path[128];
    char name[64];
    char md5[64];
    uint16_t id;
    uint8_t flag;
} ;
#define UPGRADE_FILE_REMOTE_PATH "/upgrade"


struct FwUpdater
{
    char *dev_name;
    int (*trans_func)(void *);
    int (*update_func)(void *);
    int (*update_cb)(void *);
} __attribute__((aligned(8)));

#define REGISTER_FIRMWARE_UPDATE_INTERFACE(name, trans, update, up_cb)\
    __attribute__((used, __section__("firmware_update"))) static struct FwUpdater firmware_update_##name = { \
        .dev_name = #name, \
        .trans_func = trans, \
        .update_func = update, \
        .update_cb = up_cb \
    }

#endif