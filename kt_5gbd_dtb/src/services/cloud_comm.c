#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <event2/event.h>
#include "cJSON.h"
#include "queue.h"
#include "lane_to.h"
#include "tcp_client.h"
#include "ftp_handler.h"
#include "cloud_comm.h"

extern SGData sg_data;

uint16_t checkSum_8(uint8_t *buf, uint16_t len)
{
    uint8_t i;
    uint16_t ret = 0;
    for(i=0; i<len; i++)
    {
        ret += *(buf++);
    }
//    ret = ~ret;
    return ret;
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

void init_ota_heart_beat(OtaHeartBeat *heart_beat)
{
    if (heart_beat == NULL) {
        return;
    }
    memset(heart_beat, 0, sizeof(OtaHeartBeat));
    // heart_beat->dev_addr = strudp();
    // heart_beat->usage_cpu = strdup();
    // heart_beat->up_time = strdup();
    // heart_beat->sys_time = strdup();


}

void uninit_ota_heart_beat(OtaHeartBeat *heart_beat)
{
    if (heart_beat == NULL) {
        return;
    }

    free(heart_beat->usage_cpu);
    free(heart_beat->up_time);
    free(heart_beat->sys_time);
    if (heart_beat->hw_unit != NULL) {
        free(heart_beat->hw_unit);
    }

    if (heart_beat->sw_unit != NULL) {
        free(heart_beat->sw_unit);
    }
}


cJSON *create_unit_info_object(UnitInfo * unit_info)
{
    cJSON *obj = NULL;

    obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, unit_info->unit_name, unit_info->unit_ver);
    
    return obj;
}

cJSON *create_unit_info_array(uint8_t num, UnitInfo* info)
{
    cJSON *array = NULL;
    cJSON *item = NULL;
    uint8_t i = 0;

    array = cJSON_CreateArray();
    for (i = 0; i < num; i++) {
        item = create_unit_info_object(&info[i]);
        cJSON_AddItemToArray(array, item);
    }

    return array;
}

int create_heartbeat_data(char *data)
{
    cJSON *root = NULL;
    cJSON *unit_info = NULL;
    OtaHeartBeat heart_beat;
    char *buf = NULL;
    char str[20];

    if (data == NULL) {
        return -1;
    }

    root = cJSON_CreateObject();
    init_ota_heart_beat(&heart_beat);
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    sprintf(str, "%hu", heart_beat.dev_addr);
    cJSON_AddStringToObject(root, "deviceAddress", str);
    cJSON_AddStringToObject(root, "usageCpu", heart_beat.usage_cpu);
    sprintf(str, "%u", heart_beat.usage_mem);
    cJSON_AddStringToObject(root, "usageMemory", str);
    sprintf(str, "%u", heart_beat.total_mem);
    cJSON_AddStringToObject(root, "totalMemory", str);
    sprintf(str, "%u",heart_beat.usage_disk);
    cJSON_AddStringToObject(root, "usageDisk", str);
    sprintf(str, "%u", heart_beat.total_disk);
    cJSON_AddStringToObject(root, "totalDisk", str);
    cJSON_AddStringToObject(root, "upTime", heart_beat.up_time);
    cJSON_AddStringToObject(root, "systemTime", heart_beat.sys_time);
    cJSON_AddStringToObject(root, "extendInfo", "");
    unit_info = create_unit_info_array(heart_beat.hw_unit_num, heart_beat.hw_unit);
    cJSON_AddItemToObject(root, "hardwareList", unit_info);
    unit_info = create_unit_info_array(heart_beat.sw_unit_num, heart_beat.sw_unit);
    cJSON_AddItemToObject(root, "softwareList", unit_info);
    uninit_ota_heart_beat(&heart_beat);

    buf = cJSON_Print(root);
    strncpy(data, buf, strlen(buf));
    cJSON_Delete(root);
    free(buf);

    return 0;
}
// http_post_request

// char *time_to_str(Time* t)
// {
// 	char *p;

// 	sprintf(p, "%04d-%02d-%02d %02d:%02d:%02d",
// 				t->usYear, t->ucMonth, t->ucDay, t->ucHour, t->ucMinute, t->ucSecond);

// 	return p;
// }


int create_ota_report_data(char *data)
{
    cJSON *root = NULL;
    OtaReport report;
    char buf[40];

    if (data == NULL) {
        return -1;
    }
    memset(&report, 0, sizeof(OtaReport));
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    sprintf(buf, "%hu", report.dev_addr);
    cJSON_AddStringToObject(root, "deviceAddress", buf);
    sprintf(buf, "%hu", report.task_id);
    cJSON_AddStringToObject(root, "taskId", buf);
    TIME_TO_STR(&report.up_time, buf);
    cJSON_AddStringToObject(root, "executionTime", buf);
    TIME_TO_STR(&report.report_time, buf);
    cJSON_AddStringToObject(root, "executionReport", buf);
    data = cJSON_Print(root);
    cJSON_Delete(root);

    return 0;
}


// int create_ota_update_data(char *data)
// {



// }

void ota_heartbeat_task_cb(evutil_socket_t fd, short event, void *arg)
{
    char buf[1024];
    char *resp = NULL;

    int ret = create_heartbeat_data(buf);
    if (ret) {
        return;
    }

    http_post_request(OTA_HEARTBEAT_URL, buf, &resp);
    printf("%s\n", resp);

    free(resp);

    return;
}


void ota_report_task_cb(evutil_socket_t fd, short event, void *arg)
{
    char buf[512];
    char *resp = NULL;
    int ret = create_ota_report_data(buf);
    if (ret) {
        return ;
    }
    http_post_request(OTA_HEARTBEAT_URL, buf, &resp);
    printf("%s\n", resp);

    free(resp);

    return ;
}

void nav_data_msg_task_cb(evutil_socket_t fd, short event, void *arg) 
// int proc_nav_data_msg(void *arg)
{
    MsgFramHdr *hdr = NULL;
    NAVDataSeg *nav_data = NULL;
    MsgDataFramCrc *crc = NULL;
    Time t;
    uint8_t buf[512];
    char str[50];

    if (arg == NULL) {
        return ;
    }
    // printf("nav_data_msg_task_cb...\n");
    CloundCommContext *ctx = (CloundCommContext *)arg;
    laneTo_read_nav_data(ctx->laneTo);
    // ThreadSafeQueue *send_queue = &ctx->queue;
    memset(buf, 0, sizeof(buf));
    hdr = (MsgFramHdr *)buf;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MSG_SIGN_TRANS_NAV_DATA;
    hdr->usLen = sizeof(MsgFramHdr) + sizeof(NAVDataSeg);
    // printf("hdr->usLen %d, sizeof(NAVDataSeg) %ld\n", hdr->usLen, sizeof(NAVDataSeg));
    nav_data = (NAVDataSeg *)(buf + sizeof(MsgFramHdr));
    get_system_time(&t);
    TIME_TO_STR(&t, str);
    printf("time %s, sg_data.message_id %s, sg_data.latitude %.8lf\n", str, sg_data.message_id, sg_data.latitude);
    nav_data->usDevAddr = 0;
    nav_data->usYear = sg_data.utc_year;
    nav_data->ucMonth = sg_data.utc_month;
    nav_data->ucDay = sg_data.utc_day;
    nav_data->ucHour = sg_data.utc_hour;
    nav_data->ucMinute = sg_data.utc_minutes;
    nav_data->usMilSec = sg_data.utc_millisecond; 
    nav_data->dLatitude = sg_data.latitude;
    // memcpy(&(nav_data->dLatitude[0]), &(sg_data.latitude), sizeof(nav_data->dLatitude));
    nav_data->dLongitude = sg_data.longitude;
    nav_data->fAltitude = sg_data.altitude_msl;
    nav_data->lVn = sg_data.vn;
    nav_data->lVe = sg_data.ve;
    nav_data->lVd = sg_data.vd;
    nav_data->ulSpeed = sg_data.ground_speed;
    nav_data->ulTraveDis = sg_data.traveled_distance;    
    nav_data->lRoll = sg_data.roll;
    nav_data->lPitch = sg_data.pitch;
    nav_data->lHeading = sg_data.heading;
    nav_data->usNorthPos = sg_data.north_uncertainty;
    nav_data->usEastPos = sg_data.east_uncertainty;
    nav_data->usDownPos = sg_data.down_uncertainty;
    nav_data->usVnSpeed = sg_data.vn_uncertainty;
    nav_data->usVeSpeed = sg_data.ve_uncertainty;
    nav_data->usVdSpeed = sg_data.vd_uncertainty;
    nav_data->usRollAngle = sg_data.roll_uncertainty;
    nav_data->usPitchAngle = sg_data.pitch_uncertainty;
    nav_data->usYawAngle = sg_data.yaw_uncertainty;
    nav_data->sRollMisAngle = sg_data.misalign_angle_roll;
    nav_data->sPitchMisAngle = sg_data.misalign_angle_pitch;
    nav_data->sYawMisAngle = sg_data.misalign_angle_yaw;
    nav_data->usStationID = sg_data.reference_station_id;
    nav_data->ucTimeDiff = sg_data.time_since_last_diff;
    crc = (MsgDataFramCrc *)(buf + sizeof(MsgFramHdr) + sizeof(NAVDataSeg));
    crc->usCRC = checkSum_8(buf, hdr->usLen);
    // enqueue(send_queue, buf, hdr->usLen);
    // printf("crc->usCRC 0x%x\n", crc->usCRC);
    TcpClient *client = ctx->client;
    if (client->is_connected) {
        client->ops->send(client, buf, hdr->usLen + sizeof(MsgDataFramCrc));
    }

    return;
}

void proc_message_cb(char *buf, size_t len)
{

    printf("proc_message_cb %s, %ld\n", buf, len);
}

void add_timer_task(struct event_base *base, void (task_cb)(evutil_socket_t, short, void*), uint32_t ms, void *arg)
{
    struct event *task = event_new(base, -1, EV_PERSIST, task_cb, arg);
    struct timeval tv = {ms / 1000, ms % 1000 * 1000}; 
    event_add(task, &tv);
}

void *timer_task_entry(void *arg)
{
    CloundCommContext *ctx = NULL;
    struct event_base *base = NULL;
    
    if (arg == NULL) {
        return NULL;
    }
    printf("timer_task_entry\n");
    ctx = (CloundCommContext *)arg;
    base = event_base_new();
    add_timer_task(base, nav_data_msg_task_cb, 1000, arg);
    // add_timer_task(base, ota_heartbeat_task_cb, 1000);
    ctx->base = base;
    event_base_dispatch(base);  // 启动事件循环
    
    event_base_free(base);
    return NULL;
}

// void send_msg_entry(void *arg)
// {
//     CloundCommContext *ctx = (CloundCommContext *)arg;
//     TcpClient *client = ctx->client;
//     ThreadSafeQueue *send_queue = &ctx->queue;
//     uint8_t buf[1024];
//     size_t len = 0;

//     while (ctx->running) {
//         int ret = dequeue(send_queue, buf, &len);
//         if (ret) {
//             continue;
//         }

//         client->ops->send(client, buf, len);
//         sleep(100);
//     }
// }

void clound_comm_init(CloundCommContext *ctx)
{
    // CloundCommContext *ctx = NULL;
    TcpClient *client = NULL;

    // ctx = (CloundCommContext *)malloc(sizeof(CloundCommContext));
    // if (ctx) {
    //     memset(ctx, 0, sizeof(CloundCommContext));
    // }


    ctx->running = true;
    init_queue(&ctx->queue, 512);

    client = tcp_client_create(CLOUD_SERVER_IP, CLOUD_SERVER_PORT, MAX_RECONNECT_ATTEMPTS);
    client->ops->register_cb(client, proc_message_cb);
    ctx->client = client;
    client->ops->connect(client);
    // pthread_create(&ctx->send_thread, NULL, send_msg_entry, ctx);
    ctx->laneTo = (LaneToCtx*)malloc(sizeof(LaneToCtx));
    laneTo_init(ctx->laneTo);
    pthread_create(&ctx->timer_thread, NULL, timer_task_entry, ctx);
}

void clound_comm_uninit(CloundCommContext *ctx)
{
    // CloundCommContext *ctx;
    printf("clound_comm_uninit\n");
    ctx->running = false;
    // pthread_join(ctx->send_thread, NULL);
    event_base_loopbreak(ctx->base);
    laneTo_uninit(ctx->laneTo);
    pthread_join(ctx->timer_thread, NULL);
    ctx->client->ops->disconnect(ctx->client);
    printf("clound_comm_uninit....\n");
    tcp_client_destroy(ctx->client);
    clean_queue(&ctx->queue);
}