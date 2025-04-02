#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "queue.h"
#include "mqtt_client.h"
#include "ftp_handler.h"
#include "fkz9_comm.h"


#define WAVEDAT_FILE_PATH  "/upload/cktt/wavedat/"

// uint16_t checkSum_8(uint8_t *buf, uint16_t len) //buf为数组，len为数组长度
// {
//     uint8_t i;
//     uint16_t ret = 0;
//     for(i=0; i<len; i++)
//     {
//         ret += *(buf++);
//     }
// //    ret = ~ret;
//     return ret;
// }


/* 数据终端盒到FKZ9 */
int proc_vod_file_req(void *arg)
{
    MsgFramHdr *hdr = NULL;
    VODFileReq *req = NULL;
    MsgDataFramCrc *crc = NULL;

    if (arg == NULL) {
        return -1;
    }
    hdr = (MsgFramHdr*)arg;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MSG_SIGN_VOD_FILE_REQUEST;
    hdr->usLen = sizeof(MsgFramHdr) + sizeof(VODFileReq);

    req = (VODFileReq*)(hdr + 1);
    req->usDevAddr = 0;
    req->emTransMode = TRANS_MODE_FTP;
    req->usYear = 0;
    req->ucMonth = 0;
    req->ucDay = 0;
    req->ucHour = 0;
    req->ucMinute = 0;
    strncpy(req->cFilePath, WAVEDAT_FILE_PATH, strlen(WAVEDAT_FILE_PATH));
    req->cFilePath[strlen(WAVEDAT_FILE_PATH)] = "\0";
    req->ucVerMode = 0;
    memset(req->ucRsvd, 0, sizeof(req->ucRsvd));

    crc = (MsgDataFramCrc*)(req + 1);
    crc->usCRC = checkSum_8((uint8_t*)hdr, hdr->usLen);

    return 0;
}   
/* FKZ9到数据终端盒到 */
int proc_vod_file_resp(void *arg)
{   
    VODFileResp *resp;
    uint8_t md5[32] = {0};
    char file_path[128];

    if (arg == NULL) {
        return -1;
    }

    resp = (VODFileResp*)((uint8_t*)arg + sizeof(MsgFramHdr));
    if (resp->ucIsExist == 2) {
        memcpy(md5, resp->ucMD5, sizeof(resp->ucMD5));
        strncpy(file_path, resp->cFilePath, strlen(resp->cFilePath))
        file_path[strlen(resp->cFilePath)] = '\0';
    }
    return 0;
}

/* 数据终端盒到FKZ9 */
int proc_vod_file_trans_fb(void *arg)
{
    MsgFramHdr *hdr = NULL;
    VODFileTFB *trans_fb = NULL;
    MsgDataFramCrc *crc = NULL;

    if (arg == NULL) {
        return -1;
    }

    hdr = (MsgFramHdr*)arg;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MSG_SIGN_VOD_FILE_TRANS_FB;
    hdr->usLen = sizeof(MsgFramHdr) + sizeof(VODFileTFB);
    trans_fb = (VODFileTFB*)(hdr + 1);
    trans_fb->ucResult = 0;
    trans_fb->emCode = ERR_CODE_SUCCESS;
    trans_fb->emReFlag = RETRANS_NO_NEED;
    crc = (MsgDataFramCrc*)(trans_fb + 1);
    crc->usCRC = checkSum_8((uint8_t*)hdr, hdr->usLen);

    return 0;
}
void get_system_time(Time *t)
{
    time_t rawtime;
	struct tm *timeinfo;

    if (t == NULL) {
        return;
    }

	time (&rawtime);
	timeinfo = localtime(&rawtime);

    t->usYear = timeinfo->tm_year + 1900;
    t->ucMonth = timeinfo->tm_mon + 1;
    t->ucDay = timeinfo->tm_mday;
    t->ucHour = timeinfo->tm_hour;
    t->ucMinute = timeinfo->tm_min;
    t->ucSecond = timeinfo->tm_sec;
}

/* 数据终端盒到FKZ9 */
int  proc_version_info_req(void *arg)
{
    MsgFramHdr *hdr = NULL;
    VersionInfoReq *req = NULL;
    MsgDataFramCrc *crc = NULL;

    if (arg == NULL) {
        return -1;
    }
    
    hdr = (MsgFramHdr*)arg;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MSG_SIGN_VERSION_INFO_REQUEST;
    hdr->usLen = sizeof(MsgFramHdr) + sizeof(VersionInfoReq);
    req = (VersionInfoReq*)(hdr + 1);
    req->usDevAddr = 0;
    get_system_time(&req->stTime);
    crc = (MsgDataFramCrc*)(req + 1);
    crc->usCRC = checkSum_8((uint8_t*)hdr, hdr->usLen);

    return 0;
}   

int proc_version_info_resp(void *arg)
{
    VersionInfoResp *resp = NULL;
    
    if (arg == NULL) {
        return -1;
    }
    resp = (VersionInfoResp*)((uint8_t*)arg + sizeof(MsgFramHdr));
    resp->cCPUInfo = 

}

int proc_update_pack_req(void *arg)
{
    MsgFramHdr *hdr = NULL;
    UpdatePackReq *req = NULL;
    MsgDataFramCrc *crc = NULL;
    uint8_t md5[32];
    char file_path[128];
    
    if (arg == NULL) {
        return -1;
    }
    hdr = (MsgFramHdr*)arg;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MSG_SIGN_UPDATE_PACK_REQUEST;
    hdr->usLen = sizeof(MsgFramHdr) + sizeof(UpdatePackReq);
    req = (UpdatePackReq*)(hdr + 1);
    req->usDevAddr = 0;
    req->usFileType = 0;
    req->emUpdateType = 0;
    req->emTransMode = TRANS_MODE_FTP;
    req->ucVerMode = 0;
    memcpy(req->ucMD5, md5, sizeof(md5));
    strncpy(req->cFilePath, file_path, strlen(req->cFilePath));
    req->cFilePath[strlen(req->cFilePath)] = '\0';
    crc = (MsgDataFramCrc*)(req + 1);
    crc->usCRC = checkSum_8((uint8_t*)hdr, hdr->usLen);

    return 0;
}

int proc_update_pack_resp(void *arg)
{
    UpdatePackResp *resp = NULL;

    if (arg == NULL) {
        return -1;
    }

    resp = (UpdatePackResp*)((uint8_t*)arg + sizeof(MsgFramHdr));

}   

int proc_update_report_req(void *arg)
{   
    MsgFramHdr *hdr = NULL;
    UpdateReportReq *req = NULL; 
    MsgDataFramCrc *crc = NULL;

    if (arg == NULL) {
        return -1;
    }

    req = (UpdateReportReq*)((uint8_t*)arg + sizeof(MsgFramHdr));

}

int proc_update_report_resp(void *arg)
{
    MsgFramHdr *hdr = NULL;
    UpdateReportResp *resp = NULL;
    MsgDataFramCrc *crc = NULL;
    
    if (arg == NULL) {
        return -1;
    }
    hdr = (MsgFramHdr*)arg;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MSG_SIGN_UPDATE_REPORT_RESP;
    hdr->usLen = sizeof(MsgFramHdr) + sizeof(UpdateReportResp);
    resp = (UpdateReportResp*)(hdr + 1);
    resp->usDevAddr = 0;
    resp->usFileType = 0;
    resp->emCode = ERR_CODE_SUCCESS;
    resp->emReFlag = RETRANS_NO_NEED;
    crc = (MsgDataFramCrc*)(resp + 1);
    crc->usCRC = checkSum_8((uint8_t*)hdr, hdr->usLen);

    return 0;
}

void proc_message(const char* topic, const void* payload, size_t payload_len)
{
    // AsyncMQTTClient *mqtt_client= (AsyncMQTTClient*)ctx;
    
    if (topic = NULL && payload == NULL && payload_len == 0) {
        return;
    }

    for () {

    }



}

void *send_msg_entry(void *arg)
{
    CommContext *ctx = (CommContext*)arg;
    while (1) {
        ctx->mqtt_client.publish(&ctx->mqtt_client, topic, payload, payload_len);
    }

}


void do_init(void)
{
    CommContext *ctx = malloc(sizeof(CommContext));
    
    if (malloc == NULL) {
        return ;
    }
    memset(ctx, 0, sizeof(CommContext));

    init_queue(ctx->tx_queue, MAX_MSG_SIZE);
    init_queue(ctx->re_queue, MAX_MSG_SIZE);


}