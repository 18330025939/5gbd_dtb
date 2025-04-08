#ifndef __FKZ9_COMM_H
#define __FKZ9_COMM_H

#define MAX_MSG_SIZE    1024

#define MQTT_SERVER_IP  "192.168.42.50"
#define MQTT_SERVER_PORT 1883
#define MQTT_SERVER_USERNAME "cktt"
#define MQTT_SERVER_PASSWORD "cktt"

#define MQTT_CLIENT_ID  "bd_box"

#define FTP_USERNAME   "cktt"
#define FTP_PASSWORD   "cktt"
#define FTP_CTRL_PORT       21
#define FTP_DATA_PORT       20


// #define MSG_DATA_FRAM_HDR         0xAAAA
// #pragma pack(push, 1)
// typedef struct st_MsgFramHdr
// {
//     uint16_t usHdr;       /* 帧头 */
//     uint16_t usLen;       /* 长度 */
//     uint8_t ucSign;       /* 标识 */
// } MsgFramHdr; 

// typedef struct st_MsgDataFramCrc
// {
//     uint16_t usCRC;      /* 校验 */
// } MsgDataFramCrc; 
// #pragma pack(pop)


// fkz9/设备地址/5G/file/0x15
// fkz9/设备地址/file/5G/0x16
// fkz9/设备地址/5G/file/0x17
// fkz9/设备地址/5G/OTA/0x18
// fkz9/设备地址/OTA/5G /0x19
// fkz9/设备地址/5G/OTA/0x1A
// fkz9/设备地址/OTA/5G/0x1B
// fkz9/设备地址/OTA/5G/0x1C
// fkz9/设备地址/5G/OTA/0x1D

#define MSG_SIGN_VOD_FILE_REQUEST       0x15
#define MSG_SIGN_VOD_FILE_RESPONSE      0x16
#define MSG_SIGN_VOD_FILE_TRANS_FB      0x17
#define MSG_SIGN_VERSION_INFO_REQUEST   0x18
#define MSG_SIGN_VERSION_INFO_RESPONSE  0x19
#define NSG_SIGN_UPDATE_PACK_REQUEST    0x1A
#define MSG_SIGN_UPDATE_PACK_RESPONSE   0x1B
#define MSG_SIGN_UPDATE_REPORT_REQUEST  0x1C
#define MSG_SIGN_UPDATE_REPORT_RESPONSE 0x1D

#define MQTT_HEARTBEAT_REQUEST_TOPIC "/fkz9/%d/4G/CPU/0x15"
#define MQTT_HEARTBEAT_RESPONSE_TOPIC "/fkz9/%d/4G/CPU/0x16"

#define MSG_SIGN_HEARTBEAT_REQUEST      0x15
#define MSG_SIGN_HEARTBEAT_RESPONSE     0x16

#pragma pack(push, 1)
typedef struct st_HeartBeatDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    Time     stTime;
    uint8_t  ucRsvd[8];
} HeartBeatDataSeg;
#pragma pack(pop)


typedef enum em_DataType
{
    DATA_TYPE_SIGN = 0,
    DATA_TYPE_DATA,
    DATA_TYPE_APP_ACK,

    DATA_TYE_UNKN = 0xFF
} DataType;

typedef enum em_UserPwdType
{
    USER_PWD_TYPE_FTP = 0,
    USER_PWD_TYPE_MQTT,

    USER_PWD_TYPE_UNKN = 0xFF
} UPType;

// #pragma pack(push, 1)
// typedef struct st_UserPwdDataSeg
// {
//     DataType emDataType;
//     uint16_t usDevAddr;     /* 0x0000 ～ 0x9999 */
//     UPType   emUPType;
//     char     cUser[17];
//     char     uPwd[17];
//     uint8_t  ucRsvd[3];
// } UserPwdDataSeg;
// #pragma pack(pop)

typedef enum em_TransMode
{
    TRANS_MODE_FTP = 0,
    TRANS_MODE_MQTT,

    TRANS_MODE_UNKN = 0xFF
} TransMode;

#pragma pack(push, 1)
typedef struct st_VODFileReqDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    TransMode emTransMode;
    uint16_t  usYear;
    uint8_t   ucMonth;
    uint8_t   ucDay;
    uint8_t   ucHour;
    uint8_t   ucMinute; 
    char      cFilePath[41]; // /upload/cktt/wavedat/年月日/时/点播文件名称
    uint8_t   ucVerMode;     /* 点播文件校验方法，0：MD5(默认值) */
    uint8_t   ucRsvd[6];
} VODFileReq;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct st_VODFileRespDataSeg 
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint8_t  ucIsExist;
    uint8_t  ucMD5[32];
    char     cFilePath[81];
    uint8_t  ucRsvd[5];
} VODFileResp;
#pragma pack(pop)

typedef enum em_ErrCode
{
    ERR_CODE_SUCCESS = 0,
    ERR_CODE_VER_FAIL,
    ERR_CODE_OTHER,
} ErrCode;

typedef enum em_RetransFlag
{
    RETRANS_NO_NEED = 0,
    RETRANS_NEED
} ReFlag;

#pragma pack(push, 1)
typedef struct st_VODFileTFBDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t ucResult;
    ErrCode  emCode;
    ReFlag   emReFlag;
    uint8_t  ucRsvd[3];
} VODFileTFB;

// typedef struct st_Time
// {
//     uint16_t usYear;
//     uint8_t  ucMonth;
//     uint8_t  ucDay;
//     uint8_t  ucHour;
//     uint8_t  ucMinute;
//     uint8_t  ucSecond;
// } Time;

typedef struct st_VersionInfoReqDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    Time stTime;
} VersionInfoReq;

typedef struct st_VersionInfoRespDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    Time     stCurrTime;
    Time     stUpTime;
    char     cHwDAUVer[9];
    char     cSwDAUVer[9];  
    char     cHwNUVer[9];
    char     cSwNUVer[9];  
    char     cHwCUVer[9];
    char     cSwCUVer[9];  
    char     cHwCPUVer[9];
    char     cSwCPUVer[8];  
    char     cCPUInfo[17];
    uint32_t ulTotalDiskSize;   /* 单位M */
    uint32_t ulAvailDiskSize;   /* 单位M */
    uint32_t ulTotalMemSize;
    uint32_t ulAvailMemSize;    /* 单位kb*/ 
} VersionInfoResp;
#pragma pack(pop)

typedef enum em_BoardType
{
    BOARD_TYPE_ACQ = 1,
    BOARD_TYPE_PWR,
    BOARD_TYPE_CTRL,
    BOARD_TYPE_ACQ1,
    BOARD_TYPE_UNKN = 0xFF
} BoardType;

typedef enum {
    UPDATE_TYPE_NOW = 0,
    UPDATE_TYPE_REBOOT
} UpdateType;

#pragma pack(push, 1)
typedef struct st_UpdatePackReqDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t usFileType;
    UpdateType emUpdateType;
    TransMode emTransMode;
    uint8_t   ucVerMode; 
    uint8_t   ucMD5[32];
    char      cFilePath[65];  //FKZ9:/upgrade/task_id/filename  5g:/upgrade/cktt/task_id/filename
} UpdatePackReq;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct st_UpdatePackRespDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t usFileType;
    ErrCode  emCode;
    ReFlag   emReFlag;
    uint8_t  ucRsvd[2];
} UpdatePackResp;   
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct st_UpdateReportReqDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t usFileType;
    TransMode emTransMode;
    uint8_t   ucMD5[32];
    char      cFilePath[65];
    uint8_t  ucRsvd[3];
} UpdateReportReq;

typedef struct st_UpdateReportRespDataSeg
{
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t usFileType;
    ErrCode  emCode;
    ReFlag   emReFlag;
    uint8_t  ucRsvd[2];
} UpdateReportResp;
#pragma pack(pop)

// #pragma pack(push, 1)
// typedef struct st_MsgConfirmDataSeg
// {
//     uint8_t ucFlag;
//     DataType emDataType;
//     uint8_t ucConfirmFlag;
// } MsgConfirmDataSeg;
// #pragma pack(pop)

// #pragma pack(push, 1)
// typedef struct st_BreakPointRetransDataSeg
// {
//     DataType emDataType;
//     uint16_t usStartPos;
//     uint8_t  ucDir;
//     uint16_t usReqDataLen;
//     uint8_t  ucRsvd[4];
// } BreakPointRetransDataSeg;
// #pragma pack(pop)


typedef struct st_Fkz9CommContext
{
    AsyncMQTTClient *mqtt_client;
    ThreadSafeQueue tx_queue;
    ThreadSafeQueue re_queue;
    pthread_t send_thread;
    uint16_t fkz9_dev_addr;
    pthread_t timer_thread;
    struct event_base *base;
    struct List ev_list;
} Fkz9CommContext;

void fkz9_comm_init(Fkz9CommContext *ctx);
void fkz9_comm_uninit(Fkz9CommContext *ctx);

#endif