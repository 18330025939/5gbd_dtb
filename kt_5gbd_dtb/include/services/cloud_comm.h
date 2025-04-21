#ifndef __CLOUD_COMM_H
#define __CLOUD_COMM_H

#include "tcp_client.h"
#include "lane_to.h"
#include "queue.h"
#include "list.h"


#define CLOUD_SERVER_IP   "152.136.10.158"
#define CLOUD_SERVER_PORT 3901
#define CLOUD_SERVER_USERNAME "123"
#define CLOUD_SERVER_PASSWORD "123"

#define OTA_HEARTBEAT_URL  "https://ota.cktt.com.cn/ota-server/heartbeat"
#define OTA_UPREPORT_URL   "/ota-server/submitReport"

/* WAVE文件路径格式： WAVE_FILE_PATH + 年月日 + / + 年月日时 + / + WAVE文件 */
#define WAVE_FILE_PATH "/upload/fkz9/wavedat/"

/* OTA升级报告路径格式： OTA_UPREPORT_PATH + task_id + / + filename*/
#define OTA_UPREPORT_PATH "/upgrade/cktt/upgradereport/"


#define MSG_DATA_FRAM_HDR         0xAAAA
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


typedef struct st_Time
{
    uint16_t usYear;
    uint8_t  ucMonth;
    uint8_t  ucDay;
    uint8_t  ucHour;
    uint8_t  ucMinute;
    uint8_t  ucSecond;
} Time;

#define TIME_TO_STR(t, buffer) \
    do { \
        sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", \
                (t)->usYear, (t)->ucMonth, (t)->ucDay, \
                (t)->ucHour, (t)->ucMinute, (t)->ucSecond); \
    } while (0)

/* 云平台对设备端是请求，设备端对云平台是响应*/
#define  MSG_SIGN_WAVE_FILE_REQ      0xAE
#define  MSG_SIGN_WAVE_FILE_RESP     0xAF
#define  MSG_SIGN_TRANS_NAV_DATA     0xF0

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

// struct EventList {
//     struct event *ev;
//     struct EventList *next;
// } ;

typedef struct st_MsgCommContext
{
    TcpClient *client;
    ThreadSafeQueue queue;
    pthread_t send_thread;
    pthread_t timer_thread;
    bool running;
    struct event_base *base;
    struct List ev_list;
    // struct EventList *ev_list;
    LaneToCtx *laneTo;
    struct List down_task;
    struct List upgrade_task;
} CloundCommContext;

typedef struct st_UnitInfo
{
    char unit_name[20];
    char hw_ver[20];
    char sw_ver[20];
} UnitInfo;

#define STRING_LEN_MAX 64
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
    uint16_t dev_addr;
    uint16_t task_id;
    char *time;
    char *report;
} OtaReport;

typedef enum 
{
    EVENT_TYPE_MSG = 0,
    EVENT_TYPE_TIMER,
    EVENT_TYPE_UNEXP,
    EVENT_TYPE_SIGNAL,
    EVENT_TYPE_MAX
} EventType;

typedef struct st_DownTask
{
    uint16_t id;
    char url[128];
    char md5[64];
    char type[20];
} DownTask;


typedef struct st_DownTaskList
{
    uint16_t task_num;
    struct st_DownTask *task;
} DownTaskList;

// void (*task_cb)(evutil_socket_t, short, void*);

uint16_t checkSum_8(uint8_t *buf, uint16_t len);
void get_system_time(Time *t);
void clound_comm_init(CloundCommContext *ctx);
void clound_comm_uninit(CloundCommContext *ctx);
#endif