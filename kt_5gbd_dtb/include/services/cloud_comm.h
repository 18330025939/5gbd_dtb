#ifndef __CLOUD_COMM_H
#define __CLOUD_COMM_H

#pragma pack(push, 1)
typedef struct st_MsgDataFram
{
    uint16_t usHdr;       /* 帧头 */
    uint16_t usLen;       /* 长度 */
    uint8_t ucSign;       /* 标识 */
} MsgDataFram; 

typedef struct st_MsgDataFramCrc
{
    uint16_t usCRC;      /* 校验 */
} MsgDataFramCrc; 
#pragma pack(pop)


typedef enum en_MsgSign
{
    MSG_SIGN_SG_DATA = 0xF0,
    MSG_SIGN_GNGGA_DATA,
    MSG_SIGN_GNRMC_DATA,
    MSG_SIGN_GNATT_DATA
} MsgSign;

#endif