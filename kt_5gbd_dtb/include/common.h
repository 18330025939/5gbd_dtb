#ifndef _COMMON_H_
#define _COMMON_H_

// #include "version.h"
// #include "default.h"
#include <stdint.h>
// #define F_MODE S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH|S_IWOTH|S_IXOTH|S_IXGRP

// typedef unsigned char u8;
// typedef signed char s8;
// typedef char c8;
// typedef unsigned short u16;
// typedef signed short s16;
// typedef unsigned int u32;
// typedef signed int s32;
// typedef unsigned long long u64;
// typedef signed long long s64;
// typedef int BOOL;

#define FALSE       0
#define TRUE        1


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


//#define _DEBUG_
#endif
