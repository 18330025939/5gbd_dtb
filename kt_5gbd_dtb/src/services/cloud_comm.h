#ifndef __CLOUD_COMM_H
#define __CLOUD_COMM_H

#include <stdbool.h>
#include "queue.h"
#include "list.h"


typedef struct st_TcpClient TcpClient;
typedef struct st_LaneToCtx LaneToCtx;
typedef struct st_Fx650Ctx Fx650Ctx;

#define CLOUD_SERVER_IP   "152.136.10.158"
#define CLOUD_SERVER_PORT 3901
#define CLOUD_SERVER_USERNAME "123"
#define CLOUD_SERVER_PASSWORD "123"

#define OTA_HEARTBEAT_URL  "https://ota.cktt.com.cn/ota-server/heartbeat"
#define OTA_UPREPORT_URL   "https://ota.cktt.com.cn/ota-server/submitReport"
// /* OTA升级报告路径格式： OTA_UPREPORT_PATH + task_id + / + filename*/
// #define OTA_UPREPORT_REMOTE_PATH "/home/cktt/script/"
// #define OTA_UPREPORT_LOCAL_PATH "/upgrade/cktt/upgradereport/"


/* WAVE文件路径格式： WAVE_FILE_PATH + 年月日 + / + 年月日时 + / + WAVE文件 */
#define WAVE_FILE_REMOTE_PATH "/upload/fkz9/wavedat/"
#define WAVE_FILE_LOCAL_PATH "/upload/cktt/wavedat/"

#define FTP_SERVER_URL   "ftp://192.168.10.158"

/* 升级文件路径格式：UPGRADE_FILE_PATH + task_id + / + filename */
// #define UPGRADE_FILE_REMOTE_PATH "/upgrade/"
#define UPGRADE_FILE_LOCAL_PATH "/upgrade/cktt/"

#define MSG_DATA_FRAM_HDR1         0xAAAA
#define MSG_DATA_FRAM_HDR2         0xBBBB
#pragma pack(push, 1)
typedef struct st_MsgFramHdr
{
    uint16_t usHdr;       /* 帧头 */
    uint16_t usLen;       /* 长度 */
    uint8_t ucSign;       /* 标识 */
} MsgFramHdr; 

typedef struct st_MsgDataFramCrc
{
    uint16_t usCRC;      /* 校验 */
} MsgDataFramCrc; 
#pragma pack(pop)


// typedef struct st_CustomTime
// {
//     uint16_t usYear;
//     uint8_t  ucMonth;
//     uint8_t  ucDay;
//     uint8_t  ucHour;
//     uint8_t  ucMinute;
//     uint8_t  ucSecond;
// } CustomTime;

#define TIME_TO_STR(pt, buffer) \
    do { \
        sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", \
                (pt)->usYear, (pt)->ucMonth, (pt)->ucDay, \
                (pt)->ucHour, (pt)->ucMinute, (pt)->ucSecond); \
    } while (0)

/* 云平台对设备端是请求，设备端对云平台是响应*/
#define  MSG_SIGN_WAVE_FILE_REQ      0xAE
#define  MSG_SIGN_WAVE_FILE_RESP     0xAF
#define  MSG_SIGN_TRANS_NAV_DATA     0xF0

#pragma pack(push, 1)
typedef struct st_WaveFileResp
{
    uint16_t usDevAddr;
    uint8_t ucYear;
    uint8_t ucMonth;
    uint8_t ucDay;
    uint8_t ucHour;
    uint8_t ucMinute;
    uint8_t ucCode;
} WaveFileResp;

typedef struct st_WaveFileReq
{
    uint16_t usDevAddr;
    uint8_t ucYear;
    uint8_t ucMonth;
    uint8_t ucDay;
    uint8_t ucHour;
    uint8_t ucMinute;
} WaveFileReq;
#pragma pack(pop)


/* 导航数据段 */
#pragma pack(push, 1)
typedef struct st_NAVDataSeg
{
    uint16_t usDevAddr;    /* 0x0000 ～ 0xFFFF */
    uint8_t  ucRsvd1;
    uint16_t usYear;
    uint8_t  ucMonth;
    uint8_t  ucDay; 
    uint8_t  ucHour;
    uint8_t  ucMinute;
    uint16_t usMilSec;
    double   dLatitude;
    double   dLongitude;
    float    fAltitude;
    int32_t  lVn;
    int32_t  lVe;
    int32_t  lVd;
    uint32_t ulSpeed;
    uint32_t ulTraveDis;
    int32_t  lRoll;
    int32_t  lPitch;
    int32_t  lHeading;
    uint16_t usNorthPos;
    uint16_t usEastPos;
    uint16_t usDownPos;
    uint16_t usVnSpeed;
    uint16_t usVeSpeed;
    uint16_t usVdSpeed;
    uint16_t usRollAngle;
    uint16_t usPitchAngle;
    uint16_t usYawAngle;   
    int16_t  sRollMisAngle;;
    int16_t  sPitchMisAngle;
    int16_t  sYawMisAngle;
    uint16_t usStationID;
    uint8_t  ucTimeDiff;
    uint8_t  ucRsvd[64];
} NAVDataSeg;
#pragma pack(pop)

struct MsgProcInf {
    uint8_t sign;
    int (*pFuncEntry)(void *);
    int (*pFuncCb)(void *);
} ;

#define REGISTER_MESSAGE_PROCESSING_INTERFACE(name, msg_sign, func_entry, func_cb)\
    __attribute__((used, __section__("message_processing"))) static struct MsgProcInf msg_proc_##name = { \
        .sign = msg_sign, \
        .pFuncEntry = func_entry, \
        .pFuncCb = func_cb \
    }

struct DownUpgradeTask
{
    struct List list;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t thread;
};

struct CluoudInfo {
    char ip[16];
    int port;
}

typedef struct st_MsgCommContext
{
    TcpClient *client;
    ThreadSafeQueue event_queue;
    pthread_t event_thread;
    pthread_t timer_thread;
    bool running;
    struct event_base *base;
    struct List ev_list;
    // struct EventList *ev_list;
    LaneToCtx *laneTo;
    Fx650Ctx *fx650;
    // struct List upgrade_task;
    struct DownUpgradeTask down_task;
    struct CluoudInfo cloud_info;
} CloundCommContext;

#define STRING_LEN_MAX 32
typedef struct st_UnitInfo
{
    char unit_name[STRING_LEN_MAX];
    char hw_ver[STRING_LEN_MAX];
    char sw_ver[STRING_LEN_MAX];
} UnitInfo;

typedef struct st_OtaHeartBeat
{
    char dev_addr[STRING_LEN_MAX];
    char cpu_info[STRING_LEN_MAX];
    char used_mem[STRING_LEN_MAX];
    char total_mem[STRING_LEN_MAX];
    char used_disk[STRING_LEN_MAX];
    char total_disk[STRING_LEN_MAX];
    char up_time[STRING_LEN_MAX];
    char cur_time[STRING_LEN_MAX];
    UnitInfo *units;
    uint16_t unit_num;
} OtaHeartBeat;

typedef struct st_OtaReport
{
    char dev_addr[STRING_LEN_MAX];
    char task_id[STRING_LEN_MAX];
    char time[STRING_LEN_MAX];
    char report[STRING_LEN_MAX];
} OtaReport;

typedef enum 
{
    EVENT_TYPE_MSG = 0,
    EVENT_TYPE_TIMER,
    EVENT_TYPE_UNEXP,
    EVENT_TYPE_SIGNAL,
    EVENT_TYPE_MAX
} EventType;

struct FwDownInfo
{
    uint16_t id;
    char url[128];
    char md5[64];
    char type[20];
} ;


uint16_t checkSum_8(uint8_t *buf, uint16_t len);
void clound_comm_init(CloundCommContext *ctx);
void clound_comm_uninit(CloundCommContext *ctx);
#endif /* __CLOUD_COMM_H */