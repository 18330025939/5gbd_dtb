#ifndef __CLOUD_COMM_H
#define __CLOUD_COMM_H

#include "tcp_client.h"
#include "lane_to.h"
#include "queue.h"

#define CLOUD_SERVER_IP "152.136.10.158"
#define CLOUD_SERVER_PORT 3901

#define OTA_HEARTBEAT_URL  "https://ota.cktt.com.cn/ota-server/heartbeat"
#define OTA_REPORT_URL  "/ota-server/submitReport"

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

#define  MSG_SIGN_TRANS_NAV_DATA      0xF0
// #define  MSG_SIGN_GNGGA_DATA   0xF1
// #define  MSG_SIGN_GNRMC_DATA   0xF2
// #define  MSG_SIGN_GNATT_DATA   0xF3

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

typedef struct st_MsgCommContext
{
    TcpClient *client;
    ThreadSafeQueue queue;
    pthread_t send_thread;
    pthread_t timer_thread;
    bool running;
    struct event_base *base;
    LaneToCtx *laneTo;
} CloundCommContext;

typedef struct st_UnitInfo
{
    char unit_name[20];
    char unit_ver[20];
} UnitInfo;

typedef struct st_OtaHeartBeat
{
    uint16_t dev_addr;
    char *usage_cpu;
    uint32_t usage_mem;
    uint32_t total_mem;
    uint32_t usage_disk;
    uint32_t total_disk;
    char *up_time;
    char *sys_time;
    UnitInfo *hw_unit;
    uint8_t sw_unit_num;
    UnitInfo *sw_unit;
    uint16_t hw_unit_num;
} OtaHeartBeat;

typedef struct st_OtaReport
{
    uint16_t dev_addr;
    uint16_t task_id;
    Time up_time;
    Time report_time;
} OtaReport;

typedef enum 
{
    EVENT_TYPE_MSG = 0,
    EVENT_TYPE_TIMER,
    EVENT_TYPE_UNEXP,
    EVENT_TYPE_SIGNAL,
    EVENT_TYPE_MAX
} EventType;

// void (*task_cb)(evutil_socket_t, short, void*);

uint16_t checkSum_8(uint8_t *buf, uint16_t len);
void get_system_time(Time *t);
void clound_comm_init(CloundCommContext *ctx);
void clound_comm_uninit(CloundCommContext *ctx);
#endif