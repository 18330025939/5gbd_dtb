#ifndef __CLOUD_COMM_H
#define __CLOUD_COMM_H


#define CLOUD_SERVER_IP "152.136.10.158"
#define CLOUD_SERVER_PORT 3901

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
    TcpClient client;
    ThreadSafeQueue queue;
} MsgCommContext;


#endif