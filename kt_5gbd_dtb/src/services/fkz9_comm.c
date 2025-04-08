#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <event2/event.h>
#include "queue.h"
#include "list.h"
#include "mqtt_client.h"
#include "ftp_handler.h"
#include "cloud_comm.h"
#include "fkz9_comm.h"


#define WAVEDAT_FILE_PATH  "/upload/cktt/wavedat/"

// static int _system_(const char *cmd, char *pRetMsg, int msg_len)
// {

// 	FILE *fp;
// 	int ret = -1;

// 	if (cmd == NULL || pRetMsg == NULL || msg_len <= 0)
// 		return -1;

// 	if ((fp = popen(cmd, "r")) == NULL)
// 		return -2;
// 	else {
// 		memset(pRetMsg, 0, msg_len);
// 		do{}
// 		while (fgets(pRetMsg, msg_len, fp) != NULL);
// 	}

// 	if ((ret = pclose(fp)) == -1)
// 		return -3;

// 	pRetMsg[strlen(pRetMsg)-1] = '\0';

// 	return 0;

// }


#if 0
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
    req->cFilePath[strlen(WAVEDAT_FILE_PATH)] = '\0';
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

#endif
static void on_message_cb(const char* topic, const void* payload, size_t payload_len)
{
    // AsyncMQTTClient *mqtt_client= (AsyncMQTTClient*)ctx;
    
    if (topic == NULL && payload == NULL && payload_len == 0) {
        return;
    }

    // for () {

    // }
}

static void heartbeat_req_task_cb(evutil_socket_t fd, short event, void *arg)
{
    MsgFramHdr *hdr = NULL;
    HeartBeatDataSeg *hb_data = NULL;
    MsgDataFramCrc *crc = NULL;
    AsyncMQTTClient *mqtt_client = NULL;
    uint8_t buf[100] = {0};

    if (arg == NULL) {
        return;
    }

    Fkz9CommContext *ctx = (Fkz9CommContext *)arg;
    mqtt_client = ctx->mqtt_client;
    hdr = (MsgFramHdr*)buf;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MSG_SIGN_HEARTBEAT_REQUEST;
    hdr->usLen = sizeof(MsgFramHdr) + sizeof(HeartBeatDataSeg);
    hb_data = (HeartBeatDataSeg*)(hdr + 1);
    hb_data->usDevAddr = 0;
    get_system_time(&(hb_data->stTime));
    crc = (MsgDataFramCrc*)(hb_data + 1);
    crc->usCRC = checkSum_8((uint8_t*)hdr, hdr->usLen);

    if (mqtt_client->is_conn) {
        char topic[50] = {0};
        snprintf(topic, sizeof(topic), "fkz9/%d%s", ctx->fkz9_dev_addr, MQTT_HEARTBEAT_REQUEST_TOPIC);
        mqtt_client->ops->publish(mqtt_client, topic, buf, hdr->usLen + sizeof(MsgDataFramCrc));
    }

    return;
}

static void heartbeat_resp_task_cb(evutil_socket_t fd, short event, void *arg)
{
    HeartBeatDataSeg *hb_data = NULL;

    if (arg == NULL) {
        return;
    }

    hb_data = (HeartBeatDataSeg*)((uint8_t*)arg + sizeof(MsgFramHdr));
    if (hb_data->usDevAddr == 0) {
        return;
    }

    return;
}

// void *send_msg_entry(void *arg)
// {
//     Fkz9CommContext *ctx = (Fkz9CommContext*)arg;
//     while (1) {
//         ctx->mqtt_client.publish(&ctx->mqtt_client, topic, payload, payload_len);
//     }

// }

static void add_timer_task(void *arg, void (task_cb)(evutil_socket_t, short, void*), uint32_t ms)
{
    Fkz9CommContext *ctx = NULL;
    
    if (arg == NULL) {
        return ;
    }
    
    ctx = (Fkz9CommContext *)arg;
    struct event *task = event_new(ctx->base, -1, EV_PERSIST, task_cb, arg);
    List_Insert(&ctx->ev_list, (void*)task);
    struct timeval tv = {ms / 1000, ms % 1000 * 1000}; 
    event_add(task, &tv);
}

static void *timer_task_entry(void *arg)
{
    Fkz9CommContext *ctx = NULL;
    struct event_base *base = NULL;
    struct ListNode *pNode = NULL;
    
    if (arg == NULL) {
        return NULL;
    }

    ctx = (Fkz9CommContext *)arg;
    base = event_base_new();
    ctx->base = base;
    add_timer_task(arg, heartbeat_req_task_cb, 3000);

    event_base_dispatch(base);  // 启动事件循环
    
    if (ctx->ev_list.count > 0) {
        while((pNode = List_GetHead(&ctx->ev_list)) != NULL) {
            event_free((struct event *)pNode->arg);
            List_DelHead(&ctx->ev_list);
        }
    }
    event_base_free(base);

    return NULL;
}


void fkz9_comm_init(Fkz9CommContext *ctx)
{
    // Fkz9CommContext *ctx = malloc(sizeof(Fkz9CommContext));
    
    // if (malloc == NULL) {
    //     return ;
    // }
    // memset(ctx, 0, sizeof(Fkz9CommContext));
    AsyncMQTTClient *mqtt_client = NULL;
    char url[50] = {0};

    if (ctx == NULL) {
        return;
    }

    init_queue(&ctx->tx_queue, MAX_MSG_SIZE);
    List_Init_Thread(&ctx->ev_list);
    // init_queue(ctx->re_queue, MAX_MSG_SIZE);
    snprintf(url, sizeof(url), "tcp://%s:%d", MQTT_SERVER_IP, MQTT_SERVER_PORT);
    mqtt_client = mqtt_client_create(url, MQTT_CLIENT_ID, MQTT_SERVER_USERNAME, MQTT_SERVER_PASSWORD);
    mqtt_client->ops->register_cb(mqtt_client, on_message_cb);
    int ret = mqtt_client->ops->connect(mqtt_client);
    if(ret) {
        printf("mqtt connect failed\n");
        mqtt_client_destroy(mqtt_client);
        return;
    }
    ctx->mqtt_client = mqtt_client;
    pthread_create(&ctx->timer_thread, NULL, timer_task_entry, ctx);
}

void fkz9_comm_uninit(Fkz9CommContext *ctx)
{
    AsyncMQTTClient *mqtt_client = NULL;

    if (ctx == NULL) {
        return;
    }

    event_base_loopbreak(ctx->base);
    pthread_join(ctx->timer_thread, NULL);
    mqtt_client = ctx->mqtt_client;
    // mqtt_client->ops->disconnect(mqtt_client);
    mqtt_client_destroy(mqtt_client);

    clean_queue(&ctx->tx_queue);
}