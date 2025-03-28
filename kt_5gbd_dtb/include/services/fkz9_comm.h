#ifndef __FKZ9_COMM_H
#define __FKZ9_COMM_H

#define FKZ9_SERVER_IP  "192.168.42.50"
#define MAX_MSG_SIZE    1024

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


// fkz9/设备地址/5G/file/0x15
// fkz9/设备地址/file/5G/0x16
// fkz9/设备地址/5G/file/0x17
// fkz9/设备地址/5G/OTA/0x18
// fkz9/设备地址/OTA/5G /0x19
// fkz9/设备地址/5G/OTA/0x1A
// fkz9/设备地址/OTA/5G/0x1B
// fkz9/设备地址/OTA/5G/0x1C
// fkz9/设备地址/5G/OTA/0x1D

// #define MSG_SIGN_USERNAME_PASSWORD      0xE0
#define MSG_SIGN_VOD_FILE_REQUEST       0x15
#define MSG_SIGN_VOD_FILE_RESPONSE      0x16
#define MSG_SIGN_VOD_FILE_TRANS_FB      0x17
#define MSG_SIGN_VERSION_INFO_REQUEST   0x18
#define MSG_SIGN_VERSION_INFO_RESPONSE  0x19
#define NSG_SIGN_UPDATE_PACK_REQUEST    0x1A
#define MSG_SIGN_UPDATE_PACK_RESPONSE   0x1B
#define MSG_SIGN_UPDATE_REPORT_REQUEST  0x1C
#define MSG_SIGN_UPDATE_REPORT_RESPONSE 0x1D
// #define MSG_SIGN_CONFIRMATION_MSG       0xE8
// #define MSG_SIGN_BREAKPOINT_RESUME      0xE9

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

#pragma pack(push, 1)
typedef struct st_UserPwdDataSeg
{
    DataType emDataType;
    uint16_t usDevAddr;     /* 0x0000 ～ 0x9999 */
    UPType   emUPType;
    char     cUser[17];
    char     uPwd[17];
    uint8_t  ucRsvd[3];
} UserPwdDataSeg;
#pragma pack(pop)

typedef enum em_TransMode
{
    TRANS_MODE_FTP = 0,
    TRANS_MODE_MQTT,

    TRANS_MODE_UNKN = 0xFF
} TransMode;

#pragma pack(push, 1)
typedef struct st_VODFileReqDataSeg
{
    DataType emDataType;
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    TransMode emTransMode;
    uint16_t  usYear;
    uint8_t   ucMonth;
    uint8_t   ucDay;
    uint8_t   ucHour;
    uint8_t   ucMinute; 
    char      cFilePath[17];
    uint8_t   ucVerMode;     /* 点播文件校验方法，0：MD5(默认值) */
    uint8_t   ucRsvd;
} VODFileReqDataSeg;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct st_VODFileRspDataSeg 
{
    DataType emDataType;
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint8_t  ucIsExist;
    uint8_t  ucMD5[32];
} VODFileRspDataSeg;
#pragma pack(pop)

typedef enum em_ErrCode
{
    ERR_CODE_SUCCESS = 0,
    ERR_CODE_VER_FAIL,
    ERR_CODE_OTHER,
} ErrCode;

typedef enum em_RetransFlag
{
    RETRANS_NO_NEDD = 0,
    RETRANS_NEED
} ReFlag;

#pragma pack(push, 1)
typedef struct st_VODFileTFBDataSeg
{
    DataType emDataType;
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t ucResult;
    ErrCode  emCode;
    ReFlag   emReFlag;
    uint8_t  ucRsvd;
} VODFileTFBDataSeg;

typedef struct st_Time
{
    uint16_t usYear;
    uint8_t  ucMonth;
    uint8_t  ucDay;
    uint8_t  ucHour;
    uint8_t  ucMinute;
    uint8_t  ucSecond;
} Time;

typedef struct st_VersionInfoReqDataSeg
{
    DataType emDataType;
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
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
    Time     stSwUpdata;
    Time     stCurr;
    uint16_t usManuCode;
    uint32_t ulSN;
    char     cHwPwBrdVer[9];
    char     cSwPwBrdVer[9];
    char     cRsvdBrd[18];    
} VersionInfoReqDataSeg;
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
typedef struct st_UpdateInstrDataSeg
{
    DataType emDataType;
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t usFileType;
    UpdateType emUpdateType;
    TransMode ucTransMode;
    uint8_t   ucVerMode; 
    uint8_t   ucMD5[32];
    BoardType emBoardType;
    uint16_t usManuCode;     /* 主版本+次版本，如V1.0=0x0100 */
    uint16_t usHwVersion;
    uint16_t usFwVersion;    /* 主版本+次版本+测试版，例如V1.0.0 (0x030201=3.2.1版) */
    uint32_t ulSN;
    uint8_t  ucRsvd[6];
} UpdateInstrDataSeg;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct st_UpdateInstrRsqDataSeg
{
    DataType emDataType;
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t usFileType;
    ErrCode  emCode;
    ReFlag   emReFlag;
} UpdateInstrRsqDataSeg;   
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct st_UpdateReportDataSeg
{
    DataType emDataType;
    uint16_t usDevAddr;       /* 0x0000 ～ 0x9999 */
    uint16_t usFileType;
    Time     stUpdate;
    uint8_t  ucResultLen;
    char     cResult[64];
} UpdateReportDataSeg;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct st_MsgConfirmDataSeg
{
    uint8_t ucFlag;
    DataType emDataType;
    uint8_t ucConfirmFlag;
} MsgConfirmDataSeg;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct st_BreakPointRetransDataSeg
{
    DataType emDataType;
    uint16_t usStartPos;
    uint8_t  ucDir;
    uint16_t usReqDataLen;
    uint8_t  ucRsvd[4];
} BreakPointRetransDataSeg;
#pragma pack(pop)


typedef struct st_CommContext
{
    AsyncMQTTClient mqtt_client;
    ThreadSafeQueue tx_queue;
    ThreadSafeQueue re_queue;

} CommContext;

#endif