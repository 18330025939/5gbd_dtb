#include <stdio.h>
#include <string.h>
// #include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <byteswap.h>
#include <event2/event.h>
#include "publib.h"
#include "cJSON.h"
#include "queue.h"
// #include "list.h"
#include "lane_to.h"
#include "fx650.h"
#include "tcp_client.h"
#include "ftp_handler.h"
#include "fkz9_comm.h"
#include "led.h"
#include "ssh_client.h"
#include "firmware_updater.h"
#include "cloud_comm.h"
#include "spdlog_c.h"

extern SGData sg_data;

extern struct MsgProcInf __start_message_processing;
extern struct MsgProcInf __stop_message_processing;

extern struct FwUpdater __start_firmware_update;
extern struct FwUpdater __stop_firmware_update;

CloundCommContext *gp_cloud_comm_ctx = NULL;

uint16_t checkSum_8(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    uint16_t ret = 0;
    for(i=0; i<len; i++)
    {
        ret += *(buf++);
    }
//    ret = ~ret;
    return ret;
}

void GetFileName(const char *url, char *filename)
{
    char *token = strtok((char *)url, "/");
    while (token != NULL) {
        strcpy(filename, token);
        token = strtok(NULL, "/");
    }
}

int get_ota_heartbeat_info(void *arg)
{
    SSHClient ssh_client;
    struct st_OtaHeartBeat *pHb_info = NULL;

    if (arg == NULL) {
        return -1;
    }
 
    spdlog_debug("get_ota_heart_beat_info");
#if 0 //自测使用
    pHb_info = (struct st_OtaHeartBeat*)arg;
    strcpy(pHb_info->dev_addr, "0356");
    strcpy(pHb_info->cpu_info, "10");
    strcpy(pHb_info->used_mem, "618000384");
    strcpy(pHb_info->total_mem, "4007825408");
    strcpy(pHb_info->used_disk, "5513216");
    strcpy(pHb_info->total_disk, "535805952");
    strcpy(pHb_info->up_time, "2025-05-12 15:00:00");
    strcpy(pHb_info->cur_time, "2025-05-12 15:44:00");
    pHb_info->unit_num = 4;
    pHb_info->units = (UnitInfo *)malloc(sizeof(struct st_UnitInfo) * pHb_info->unit_num);
    strcpy(pHb_info->units[0].hw_ver, "2");
    strcpy(pHb_info->units[0].sw_ver, "35");
    strcpy(pHb_info->units[0].unit_name, "中央处理单元");
    strcpy(pHb_info->units[1].hw_ver, "10");
    strcpy(pHb_info->units[1].sw_ver, "80");
    strcpy(pHb_info->units[1].unit_name, "采集单元");
    strcpy(pHb_info->units[2].hw_ver, "2");
    strcpy(pHb_info->units[2].sw_ver, "80");
    strcpy(pHb_info->units[2].unit_name, "控制单元");
    strcpy(pHb_info->units[3].hw_ver, "1");
    strcpy(pHb_info->units[3].sw_ver, "90");
    strcpy(pHb_info->units[3].unit_name, "网络单元");
#endif
#if 0 //测试使用
    pHb_info = (struct st_OtaHeartBeat*)arg;
    SSHClient_Init(&ssh_client, TEST_SERVER_IP, TEST_SERVER_USERNAME, TEST_SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "TEST ssh_client.connect failed.\n");
        return -1;
    }
    char resp[4096];
    ret = ssh_client.execute(&ssh_client, "ls /home/lrj -al", resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute get_ota_heartbeat_info failed.\n");
        return -1;
    }
    printf("resp of the cmd ls /home/lrj -al %s\n", resp);

    char file_path[256] = {'\0'};
    char local_path[256] = {'\0'};
    snprintf(file_path, sizeof(file_path), "/home/lrj/test.log");
    snprintf(local_path, sizeof(local_path), "/home/rk/test.log");
    ret = ssh_client.download_file(&ssh_client, file_path, local_path);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.download_file /home/lrj/test.log failed.\n");
        return -1;
    }

    snprintf(file_path, sizeof(file_path), "/home/lrj/rk.log");
    snprintf(local_path, sizeof(local_path), "/home/rk/rk.log");
    ret = ssh_client.upload_file(&ssh_client, local_path, file_path);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.upload_file /home/rk/rk.log failed.\n");
        return -1;
    }
#endif
#if 1
    pHb_info = (struct st_OtaHeartBeat*)arg;
    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.connect failed.");
        return -1;
    }


    char resp[512] = {0};
    ret = ssh_client.execute(&ssh_client, "bash /home/cktt/script/updater.sh base_info", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute updater.sh base_info failed.");
        return -1;
    }

    sscanf(resp, " %[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",
            pHb_info->dev_addr, pHb_info->cpu_info, 
            pHb_info->total_disk, pHb_info->used_disk, 
            pHb_info->total_mem, pHb_info->used_mem,
            pHb_info->up_time, pHb_info->cur_time);
    
    if (strncmp(pHb_info->up_time, gp_cloud_comm_ctx->base_info->up_time, strlen(pHb_info->up_time))) {
        memset(gp_cloud_comm_ctx->base_info->up_time, 0, sizeof(gp_cloud_comm_ctx->base_info->up_time));
        strncpy(gp_cloud_comm_ctx->base_info->up_time, pHb_info->up_time, strlen(pHb_info->up_time));
        gp_cloud_comm_ctx->base_info->up_time[strlen(pHb_info->up_time)] = '\0';
    }
    memset(resp, 0, sizeof(resp));
    ret = ssh_client.execute(&ssh_client, "bash /home/cktt/script/updater.sh unit_info", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute updater.sh unit_info failed.");
        return -1;
    }
    
    char tmp_resp[256] = {0};
    strcpy(tmp_resp, resp);
    char *token = strtok(resp, ";");
    while (token != NULL) {
        pHb_info->unit_num++;
        token = strtok(NULL, ";");
    }
    pHb_info->units = (UnitInfo *)malloc(sizeof(struct st_UnitInfo) * pHb_info->unit_num);
    
    int i = 0;
    token = strtok(tmp_resp, ";");
    while (token != NULL) {
        sscanf(token, "%[^:]:%[^,],%s", pHb_info->units[i].unit_name,
                pHb_info->units[i].sw_ver, pHb_info->units[i].hw_ver);
        i++;
        token = strtok(NULL, ";");
    }

    SSHClient_Destroy(&ssh_client);
#endif
    return 0;
}

cJSON *create_unit_info_object(UnitInfo * unit_info, uint8_t type)
{
    cJSON *obj = NULL;
    char version[64] = {0};

    obj = cJSON_CreateObject();
    if (type == 1) {
        cJSON_AddStringToObject(obj, "software", unit_info->unit_name);
        snprintf(version, sizeof(version), "V%s", unit_info->sw_ver);
    } else {
        cJSON_AddStringToObject(obj, "hardware", unit_info->unit_name);
        snprintf(version, sizeof(version), "V%s", unit_info->hw_ver);
    }
    cJSON_AddStringToObject(obj, "version", version);
    
    return obj;
}

cJSON *create_unit_info_array(uint8_t num, UnitInfo* info, uint8_t type)
{
    cJSON *array = NULL;
    cJSON *item = NULL;
    uint8_t i = 0;

    array = cJSON_CreateArray();
    for (i = 0; i < num; i++) {
        item = create_unit_info_object(&info[i], type);
        cJSON_AddItemToArray(array, item);
    }

    return array;
}

int create_ota_heartbeat_data(char *data)
{
    cJSON *root = NULL;
    cJSON *unit_info = NULL;
    OtaHeartBeat heart_beat;
    char *buf = NULL;
    // char str[20];

    if (data == NULL) {
        return -1;
    }
 
    spdlog_debug("create_ota_heartbeat_data");
    memset(&(heart_beat), 0x00, sizeof(OtaHeartBeat));
    int ret = get_ota_heartbeat_info(&heart_beat);
    if (ret) {
        spdlog_error("get_ota_hearbeat_info error");
        return -1;
    }
    root = cJSON_CreateObject(); 
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    cJSON_AddStringToObject(root, "deviceAddress", heart_beat.dev_addr);
    cJSON_AddStringToObject(root, "usageCpu", heart_beat.cpu_info);
    cJSON_AddStringToObject(root, "usageMemory", heart_beat.used_mem);
    cJSON_AddStringToObject(root, "totalMemory", heart_beat.total_mem);
    cJSON_AddStringToObject(root, "usageDisk", heart_beat.used_disk);
    cJSON_AddStringToObject(root, "totalDisk", heart_beat.total_disk);
    cJSON_AddStringToObject(root, "upTime", heart_beat.up_time);
    cJSON_AddStringToObject(root, "systemTime", heart_beat.cur_time);
    cJSON_AddStringToObject(root, "extendInfo", "");
    unit_info = create_unit_info_array(heart_beat.unit_num, heart_beat.units, 0);
    cJSON_AddItemToObject(root, "hardwareList", unit_info);
    unit_info = create_unit_info_array(heart_beat.unit_num, heart_beat.units, 1);
    cJSON_AddItemToObject(root, "softwareList", unit_info);
 
    buf = cJSON_Print(root);
    // spdlog_debug("ota heart beat data: %s", buf);
    strncpy(data, buf, strlen(buf));
    cJSON_Delete(root);
    free(buf);
    free(heart_beat.units);

    return 0;
}

int get_ota_report_info(struct FwUpdateInfo *info, void *arg)
{
    struct st_OtaReport *pReport = NULL;

    if (arg == NULL) {
        return -1;
    }

    pReport = (struct st_OtaReport*)arg;
    snprintf(pReport->dev_addr, sizeof(pReport->dev_addr), "%d", gp_cloud_comm_ctx->base_info->dev_addr);
    pReport->task_id = info->id;
    pReport->time = info->resp_info.time;
    pReport->report = info->resp_info.report; 

    return 0;
}

int create_ota_report_data(struct FwUpdateInfo *info, char *data)
{
    cJSON *root = NULL;
    OtaReport report;
    char *buf = NULL;

    if (data == NULL) {
         return -1;
    }

    // spdlog_debug("create_ota_report_data");
    memset(&report, 0, sizeof(OtaReport));
    int ret = get_ota_report_info(info, (void *)&report);
    if (ret != 0) {
        return -1;
    }
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    cJSON_AddStringToObject(root, "deviceAddress", report.dev_addr);
    cJSON_AddNumberToObject(root, "taskId", report.task_id);
    cJSON_AddStringToObject(root, "executionTime", report.time);
    cJSON_AddStringToObject(root, "executionReport", report.report);
    buf = cJSON_Print(root);
    strncpy(data, buf, strlen(buf));
    // spdlog_debug("ota report data: %s", buf);
    cJSON_Delete(root);
    free(buf);

    return 0;
}

int do_upgrade_firmware(struct FwUpdateInfo *pInfo)
{
    struct FwUpdater *start = &__start_firmware_update;
    struct FwUpdater *fw_up = NULL;

    for (; start != &__stop_firmware_update; start++) {
        if (strcmp(start->dev_name, "fkz9") == 0) {
            fw_up = start;
            break;
        }
    }

    int ret = fw_up->trans_func((void *)pInfo);
    if (ret) {
        spdlog_error("fw_up->trans_func failed.");
        return ret;
    }
    if (fw_up->update_func != NULL) {
        ret = fw_up->update_func((void *)pInfo);
        if (ret) {
            spdlog_error("up->update_func failed.");
        } else {
            if (fw_up->update_cb != NULL) {
                fw_up->update_cb((void *)pInfo);
            }
        }
    }

    return ret;
}

void do_ota_report(struct FwUpdateInfo *info)
{
    char buf[1024*10] = {0};
    char *resp = NULL;
    
    int ret = create_ota_report_data(info, buf);
    if (ret) {
        return ;
    }
    http_post_request(OTA_UPREPORT_URL, buf, &resp);
    spdlog_debug("do_ota_report %s", resp);
    
    if (resp != NULL) {
        free(resp);
    }
}

int do_downlaod_firmware(struct List *task_list)
{
    struct FwDownInfo *pInfo = NULL;
    struct ListNode *pNode = NULL;
    char file_name[64] = {0};
    char local_path[128] = {0};
    char remp_url[128] = {0};
    char cmd[64] = {0};
    struct FwUpdateInfo up_info;
    CustomTime t;
    int ret = 0;

    if (task_list == NULL) {
        return -1;
    }

    if (task_list->count > 0) {
        while((pNode = List_GetHead(task_list)) != NULL) {
            pInfo = (struct FwDownInfo *)pNode->arg;

            memset(&up_info, 0, sizeof(struct FwUpdateInfo));
            strcpy(remp_url, pInfo->url);
            GetFileName(remp_url, file_name);
            snprintf(local_path, sizeof(local_path), "%s%d/%s", UPGRADE_FILE_LOCAL_PATH, pInfo->id, file_name);
            if (pInfo->flag == FW_NEED_DOWNLOAD) {
                // strcpy(remp_url, pInfo->url);
                // GetFileName(remp_url, file_name);
                snprintf(cmd, sizeof(cmd), "mkdir -p %s%d", UPGRADE_FILE_LOCAL_PATH, pInfo->id);
                _system_(cmd, NULL, 0);
                // snprintf(local_path, sizeof(local_path), "%s%d/%s", UPGRADE_FILE_LOCAL_PATH, pInfo->id, file_name);
                get_system_time(&t);
                snprintf(up_info.log_path, sizeof(up_info.log_path), "%s%d/%04d_%d_%04d%02d%02d%02d%02d.log", UPGRADE_FILE_LOCAL_PATH, pInfo->id, gp_cloud_comm_ctx->base_info->dev_addr,
                            pInfo->id, t.usYear, t.ucMonth, t.ucDay, t.ucHour, t.ucMinute); 
                UPDATE_LOG_FMT(up_info.log_path, "%04d-%02d-%02d %02d:%02d:%02d upgrade mission %d\n", t.usYear, t.ucMonth, t.ucDay, t.ucHour, t.ucMinute, t.ucSecond, pInfo->id);
                UPDATE_LOG_FMT(up_info.log_path, "start to download ota file %s\n", file_name);
                ret = ftp_download(pInfo->url, local_path, NULL, CLOUD_SERVER_USERNAME, CLOUD_SERVER_PASSWORD);
                if (!ret) {
                    UPDATE_LOG_FMT(up_info.log_path, "download ota file %s success\n", file_name);
                } else {
                    UPDATE_LOG_FMT(up_info.log_path, "task %d download file failed\n", pInfo->id);
                }
                if (strstr(pInfo->type, OTA_TASK_NEXT) != NULL) {
                    UPDATE_LOG_FMT(MISSION_FILE_LOCAL_PATH, "%d,%s,%s,%s,%s,\n", pInfo->id, pInfo->url, pInfo->md5, pInfo->type, gp_cloud_comm_ctx->base_info->up_time);
                }
                strcpy(up_info.type, pInfo->type);
            } else {
                snprintf(cmd, sizeof(cmd), "find %s%d/ -name \"%04d_%d*.log\" | tr -d '\n\r'", UPGRADE_FILE_LOCAL_PATH, pInfo->id, gp_cloud_comm_ctx->base_info->dev_addr, pInfo->id);
                _system_(cmd, up_info.log_path, sizeof(up_info.log_path));
                get_system_time(&t);
                UPDATE_LOG_FMT(up_info.log_path, "%04d-%02d-%02d %02d:%02d:%02d start to excute mission %d %s\n", t.usYear, t.ucMonth, t.ucDay, t.ucHour, t.ucMinute, t.ucSecond, pInfo->id, up_info.log_path);
                snprintf(up_info.type, sizeof(up_info.type), "%s", OTA_TASK_REBOOTED);
            }
            //通知去执行更新    
            // strcpy(up_info.type, pInfo->type);
            up_info.id = pInfo->id;
            strncpy(up_info.md5, pInfo->md5, sizeof(up_info.md5));
            strncpy(up_info.path, local_path, sizeof(up_info.path));
            strncpy(up_info.name, file_name, sizeof(up_info.name));
            ret = do_upgrade_firmware(&up_info);
            if (!ret) {
                do_ota_report(&up_info);
            }

            free(pInfo);
            List_DelHead(task_list);
        }
    }

    return 0;
}

void *download_upgrade_entry(void *arg)
{
    struct DownUpgradeTask *pTask = NULL;
    CloundCommContext *ctx = NULL;

    if (arg == NULL) {
        return NULL;
    }

    ctx = (CloundCommContext *)arg;
    pTask = &(ctx->down_task);
    while (ctx->running) {
        pthread_mutex_lock(&pTask->mutex);
        while (&pTask->list.count == 0) {
            pthread_cond_wait(&(pTask->cond), &(pTask->mutex));
        }
        do_downlaod_firmware(&pTask->list);
        pthread_mutex_unlock(&pTask->mutex);
    }

    return NULL;
}

int ota_heartbeat_resp_parse(struct List *task_list, char *respond)
{
    // 解析 JSON 数据
    cJSON *root = cJSON_Parse(respond);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            spdlog_error("Error before: %s", error_ptr);
        }
        return -1;
    }

    // 提取 code 字段
    cJSON *code_obj = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (cJSON_IsString(code_obj) && (code_obj->valuestring != NULL)) {
        const char *code = code_obj->valuestring;
        if (strcmp(code, "SUCCESS") == 0) {
            spdlog_info("send heartbeat success and receive respond.");
        } else {
            spdlog_error("send heartbeat failed and respond %s %s.", code, respond);
            return -1;
        }
    }

    // 提取 taskList 字段
    cJSON *task_list_obj = cJSON_GetObjectItemCaseSensitive(root, "taskList");
    if (cJSON_IsArray(task_list_obj)) {
        int length = cJSON_GetArraySize(task_list_obj);
        if (length == 0) {
            spdlog_debug("heartbeat end");
        } else {
            for (int i = 0; i < length; i++) {
                cJSON *task_obj = cJSON_GetArrayItem(task_list_obj, i);
                cJSON *id_obj = cJSON_GetObjectItemCaseSensitive(task_obj, "taskId");
                cJSON *url_obj = cJSON_GetObjectItemCaseSensitive(task_obj, "artifactUrl");
                cJSON *md5_obj = cJSON_GetObjectItemCaseSensitive(task_obj, "artifactMd5");
                cJSON *type_obj = cJSON_GetObjectItemCaseSensitive(task_obj, "taskType");

                if (cJSON_IsNumber(id_obj) && cJSON_IsString(url_obj) && cJSON_IsString(md5_obj) && cJSON_IsString(type_obj)) {
                    struct FwDownInfo *pInfo = (struct FwDownInfo *)malloc(sizeof(struct FwDownInfo));
                    pInfo->id = id_obj->valueint;
                    strcpy(pInfo->url, url_obj->valuestring);
                    strcpy(pInfo->md5, md5_obj->valuestring);
                    strcpy(pInfo->type,type_obj->valuestring);
                    pInfo->flag = FW_NEED_DOWNLOAD;
                    List_Insert(task_list, (void*)pInfo);

                    spdlog_debug("pInfo ID: %d, URL: %s, MD5: %s, Type: %s\n", pInfo->id, pInfo->url, pInfo->md5, pInfo->type);
                }
            }
        }
    }

    return 0;
}

void ota_heartbeat_task_cb(evutil_socket_t fd, short event, void *arg)
{
    CloundCommContext *ctx = NULL;
    char buf[1024] = {0};
    char *resp = NULL;

    int ret = create_ota_heartbeat_data(buf);
    if (ret) {
        return;
    }
    // spdlog_info("ota_heartbeat : %s", buf);
    ret = http_post_request(OTA_HEARTBEAT_URL, buf, &resp);
    spdlog_debug("resp to ota_heartbeat_req: %s, %d.", resp, ret);
    if (ret == 0) {
        ctx = (CloundCommContext *)arg;
        pthread_mutex_lock(&ctx->down_task.mutex);
        ret = ota_heartbeat_resp_parse(&ctx->down_task.list, resp);
        if (ret == 0) {
            //通知去下载
            pthread_cond_signal(&ctx->down_task.cond);
        }   
        pthread_mutex_unlock(&ctx->down_task.mutex);
    }

    // if (buf != NULL)
        // free(buf);
    if (resp != NULL) 
        free(resp);
}

void nav_data_msg_task_cb(evutil_socket_t fd, short event, void *arg) 
{
    MsgFramHdr *hdr = NULL;
    NAVDataSeg *nav_data = NULL;
    MsgDataFramCrc *crc = NULL;
    // CustomTime t;
    uint8_t buf[512];
    // char str[50];

    if (arg == NULL) {
        return ;
    }
    
    RUN_LED_TOGGLE();
    CloundCommContext *ctx = (CloundCommContext *)arg;
    laneTo_read_nav_data(&ctx->laneTo);
    memset(buf, 0, sizeof(buf));
    hdr = (MsgFramHdr *)buf;
    hdr->usHdr = MSG_DATA_FRAM_HDR1;
    hdr->ucSign = MSG_SIGN_TRANS_NAV_DATA;
    uint16_t len = sizeof(MsgFramHdr) + sizeof(NAVDataSeg) + sizeof(MsgDataFramCrc);
    hdr->usLen = bswap_16(len);
    nav_data = (NAVDataSeg *)(buf + sizeof(MsgFramHdr));
    nav_data->usDevAddr = 0;
    nav_data->usYear = bswap_16(sg_data.utc_year);
    nav_data->ucMonth = sg_data.utc_month;
    nav_data->ucDay = sg_data.utc_day;
    nav_data->ucHour = sg_data.utc_hour;
    nav_data->ucMinute = sg_data.utc_minutes;
    nav_data->usMilSec = bswap_16(sg_data.utc_millisecond); 
    nav_data->dLatitude = bswap_64(sg_data.latitude);
    nav_data->dLongitude = bswap_64(sg_data.longitude);
    nav_data->fAltitude = bswap_32(sg_data.altitude_msl);
    nav_data->lVn = bswap_32(sg_data.vn);
    nav_data->lVe = bswap_32(sg_data.ve);
    nav_data->lVd = bswap_32(sg_data.vd);
    nav_data->ulSpeed = bswap_32(sg_data.ground_speed);
    nav_data->ulTraveDis = bswap_32(sg_data.traveled_distance);    
    nav_data->lRoll = bswap_32(sg_data.roll);
    nav_data->lPitch = bswap_32(sg_data.pitch);
    nav_data->lHeading = bswap_32(sg_data.heading);
    nav_data->usNorthPos = bswap_16(sg_data.north_uncertainty);
    nav_data->usEastPos = bswap_16(sg_data.east_uncertainty);
    nav_data->usDownPos = bswap_16(sg_data.down_uncertainty);
    nav_data->usVnSpeed = bswap_16(sg_data.vn_uncertainty);
    nav_data->usVeSpeed = bswap_16(sg_data.ve_uncertainty);
    nav_data->usVdSpeed = bswap_16(sg_data.vd_uncertainty);
    nav_data->usRollAngle = bswap_16(sg_data.roll_uncertainty);
    nav_data->usPitchAngle = bswap_16(sg_data.pitch_uncertainty);
    nav_data->usYawAngle = bswap_16(sg_data.yaw_uncertainty);
    nav_data->sRollMisAngle = bswap_16(sg_data.misalign_angle_roll);
    nav_data->sPitchMisAngle = bswap_16(sg_data.misalign_angle_pitch);
    nav_data->sYawMisAngle = bswap_16(sg_data.misalign_angle_yaw);
    nav_data->usStationID = bswap_16(sg_data.reference_station_id);
    nav_data->ucTimeDiff = bswap_16(sg_data.time_since_last_diff);
    crc = (MsgDataFramCrc *)(buf + sizeof(MsgFramHdr) + sizeof(NAVDataSeg));
    crc->usCRC = checkSum_8(buf, len - sizeof(MsgDataFramCrc));
    crc->usCRC = bswap_16(crc->usCRC);

    TcpClient *client = ctx->client;
    client->ops->send(client, buf, len);

    return;
}


void reboot_upgrade_task_cb(evutil_socket_t fd, short event, void *arg) 
{
    CloundCommContext *ctx = NULL;
    char uptime[30] = {0};
    char cmd[128] = {0};
    char resp[256] = {0};
    uint8_t num = 0;
    struct FwDownInfo info;

    if (arg == NULL) {
        return ;
    }

    ctx = (CloundCommContext *)arg;

    int ret = is_file_empty(MISSION_FILE_LOCAL_PATH);
    if (!ret) {
        snprintf(cmd, sizeof(cmd), "wc -l %s", MISSION_FILE_LOCAL_PATH);
        _system_(cmd, resp, sizeof(resp));
        sscanf(resp, "%hhd", &num);
        snprintf(cmd, sizeof(cmd), "sed -n '%dp' %s", 1, MISSION_FILE_LOCAL_PATH);
        _system_(cmd, resp, sizeof(resp));
        sscanf(resp, "%hd,%[^,],%[^,],%[^,],%[^,],", &info.id, info.url, info.md5, info.type, uptime);
        // spdlog_info("gp_cloud_comm_ctx->base_info->up_time: %s, %ld, uptime:%s, %ld", gp_cloud_comm_ctx->base_info->up_time, strlen(gp_cloud_comm_ctx->base_info->up_time), uptime, strlen(uptime));
        if (strncmp(uptime, gp_cloud_comm_ctx->base_info->up_time, strlen(uptime))) {
            spdlog_info("gp_cloud_comm_ctx->base_info->up_time: %s, %ld, uptime:%s, %ld", gp_cloud_comm_ctx->base_info->up_time, strlen(gp_cloud_comm_ctx->base_info->up_time), uptime, strlen(uptime));
            pthread_mutex_lock(&ctx->down_task.mutex);
            for (int i = 0; i < num; i++) {
                struct FwDownInfo *pInfo = (struct FwDownInfo *)malloc(sizeof(struct FwDownInfo));
                snprintf(cmd, sizeof(cmd), "sed -n '%dp' %s", i + 1, MISSION_FILE_LOCAL_PATH);
                _system_(cmd, resp, sizeof(resp));
                sscanf(resp, "%hd,%[^,],%[^,],%[^,],%[^,],", &pInfo->id, pInfo->url, pInfo->md5, pInfo->type, uptime);
                pInfo->flag = 0;
                List_Insert(&ctx->down_task.list, (void*)pInfo);
            } 
            pthread_cond_signal(&ctx->down_task.cond);
            pthread_mutex_unlock(&ctx->down_task.mutex);
            #if 1
            snprintf(cmd, sizeof(cmd), "rm -rf %s", MISSION_FILE_LOCAL_PATH);
            _system_(cmd, NULL, 0);
            #endif
        }
    }    

    ret = file_exists(SELF_UPGRADE_FILE_PATH);
    if (ret) {
        spdlog_info("self upgrade.....");
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "bash %s &", SELF_UPGRADE_FILE_PATH);
        system(cmd);
    }
}

int func_wave_file_resp(void *arg)
{
    MsgFramHdr *pHdr = NULL;
    WaveFileResp *pResp = NULL;
    MsgDataFramCrc *pCrc = NULL;
    uint8_t buf[64];
    TcpClient *client = NULL;
    CloundCommContext *ctx = NULL; 

    if (arg  == NULL) {
        return -1;
    }

    spdlog_info("func_wave_file_resp");
    ctx = (CloundCommContext *)arg;
    pHdr = (MsgFramHdr *)buf;
    pHdr->usHdr = MSG_DATA_FRAM_HDR1;
    pHdr->ucSign = MSG_SIGN_WAVE_FILE_RESP;
    uint16_t len = sizeof(MsgFramHdr) + sizeof(WaveFileResp) + sizeof(MsgDataFramCrc);
    pHdr->usLen = bswap_16(len);    
    pResp = (WaveFileResp *)(buf + sizeof(MsgFramHdr));
    // pResp->usDevAddr = bswap_16(CLIENT_DEV_ADDR);
    db_to_bcd(ctx->base_info->dev_addr, &pResp->usDevAddr);
    CustomTime t;
    get_system_time(&t);
    pResp->ucYear = byte_to_bcd(t.usYear - 2000);
    pResp->ucMonth = byte_to_bcd(t.ucMonth);
    pResp->ucDay = byte_to_bcd(t.ucDay);
    pResp->ucHour = byte_to_bcd(t.ucHour);
    pResp->ucMinute = byte_to_bcd(t.ucMinute);
    pResp->ucCode = 0;

    pCrc = (MsgDataFramCrc *)(buf + sizeof(MsgFramHdr) + sizeof(WaveFileResp));
    pCrc->usCRC = checkSum_8((uint8_t *)pHdr, len - sizeof(MsgDataFramCrc));
    pCrc->usCRC = bswap_16(pCrc->usCRC);

    client = ctx->client;
    client->ops->send(client, buf, len);

    return 0;
}

int func_wave_file_req(void *arg)
{
    WaveFileReq *pReq = NULL;
    SSHClient ssh_client;

    if (arg == NULL) {
        return -1;
    }
    
    spdlog_debug("func_wave_file_req.");
    pReq = (WaveFileReq *)((uint8_t *)arg + sizeof(MsgFramHdr));
    int dev_addr = bcd_to_db(pReq->usDevAddr);
    int year = bcd_to_byte(pReq->ucYear) + 2000;
    int month = bcd_to_byte(pReq->ucMonth);
    int day = bcd_to_byte(pReq->ucDay);
    int hour = bcd_to_byte(pReq->ucHour);
    int minute = bcd_to_byte(pReq->ucMinute);

    char r_folder[128] = {0};
    snprintf(r_folder, sizeof(r_folder), "%s%04d%02d%02d/%04d%02d%02d%02d/", WAVE_FILE_REMOTE_PATH, year, month, day, year, month, day, hour);

    char file_name[64] = {0};
    snprintf(file_name, sizeof(file_name), "%04d-wavedat-%04d%02d%02d%02d%02d.dat.gz", dev_addr, year, month, day, hour, minute);

    char l_path[128] = {0};
    snprintf(l_path, sizeof(l_path), "%s%04d%02d%02d%02d/%s", WAVE_FILE_LOCAL_PATH, year, month, day, hour, file_name);

    char r_path[256] = {0};
    snprintf(r_path, sizeof(r_path), "%s%s", r_folder, file_name);

    char cmd[140] = {0};
    snprintf(cmd, sizeof(cmd), "mkdir -p %s%04d%02d%02d%02d", WAVE_FILE_LOCAL_PATH, year, month, day, hour);
    _system_(cmd, NULL, 0);

    spdlog_info("l_path:%s, r_path:%s", l_path, r_path);

    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.connect failed.");
        return -1;
    }

    ret = ssh_client.download_file(&ssh_client, r_path, l_path);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.download_file cp_wave_file_from_fkz9 failed.");
        return -1;
    }

    SSHClient_Destroy(&ssh_client);

    char remote_path[128] = {0};
    // char local_path[128];
    // uint16_t dev_addr = CLIENT_DEV_ADDR;
    snprintf(remote_path, sizeof(remote_path), "/%04d/wavefile/%04d%02d/%02d/%02d/%04d-wavedat-%04d%02d%02d%02d%02d.dat.gz", dev_addr, year, month, day, 
                hour, dev_addr, year, month, day, hour, minute);
    ret = ftp_upload(FTP_SERVER_URL, l_path, remote_path, FTP_SERVER_USER, FTP_SERVER_PASS);
    if (ret != 0) {
        spdlog_info("ftp_upload error");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "rm -rf /upload");
    _system_(cmd, NULL, 0);

    return 0;
}

REGISTER_MESSAGE_PROCESSING_INTERFACE(wave_file, 174, func_wave_file_req, func_wave_file_resp);

void proc_message_cb(char *buf, size_t len)
{
    if (buf == NULL || len == 0) {
        return ;
    }

    spdlog_debug("proc_message_cb %ld.", len);
    enqueue(&gp_cloud_comm_ctx->event_queue, (uint8_t *)buf, len);
}


void *cloud_event_task_entry(void *arg)
{
    CloundCommContext *ctx = NULL;
    uint8_t buf[256];
    size_t len = 0;
    MsgFramHdr *pHdr = NULL;
    MsgDataFramCrc *pCrc = NULL;
    struct MsgProcInf *start = &__start_message_processing;
    struct MsgProcInf *end = &__stop_message_processing;

    if (arg == NULL) {
        return NULL;
    }

    ctx = (CloundCommContext *)arg;
    while (ctx->running) {
        if (dequeue(&ctx->event_queue, buf, &len)) {
            continue;
        }
        pHdr = (MsgFramHdr *)buf;
        uint16_t crc = checkSum_8((uint8_t *)buf, bswap_16(pHdr->usLen) - sizeof(MsgDataFramCrc));
        pCrc = (MsgDataFramCrc *)(buf + bswap_16(pHdr->usLen) - sizeof(MsgDataFramCrc));
        spdlog_debug("cloud_recv: pHdr->usHdr=0x%x, pHdr->ucSign=0x%x, pCrc->usCRC=0x%x, crc=0x%x.", bswap_16(pHdr->usHdr), pHdr->ucSign, bswap_16(pCrc->usCRC), crc);
        if ((pHdr->usHdr != MSG_DATA_FRAM_HDR1 && pHdr->usHdr != MSG_DATA_FRAM_HDR2) || crc != bswap_16(pCrc->usCRC)) {
            continue ;
        }

        for (; start != end; start++) {
            if (start->sign == pHdr->ucSign) {
                int ret = start->pFuncEntry(buf);
                if (ret == 0 && start->pFuncCb != NULL) {
                    start->pFuncCb(arg);
                }
                break;
            }
        }
    }
    
    return NULL;
}

void cloud_add_timer_task(void *arg, void (task_cb)(evutil_socket_t, short, void*), uint32_t ms)
{
    CloundCommContext *ctx = NULL;
    
    if (arg == NULL || task_cb == NULL) {
        return ;
    }
    
    ctx = (CloundCommContext *)arg;
    struct event *task = event_new(ctx->base, -1, EV_PERSIST, task_cb, arg);
    List_Insert(&ctx->ev_list, (void*)task);
    struct timeval tv = {ms / 1000, ms % 1000 * 1000}; 
    event_add(task, &tv);
}

void *cloud_timer_task_entry(void *arg)
{
    CloundCommContext *ctx = NULL;
    struct event_base *base = NULL;
    struct ListNode *pNode = NULL;
    
    if (arg == NULL) {
        return NULL;
    }

    ctx = (CloundCommContext *)arg;
    base = event_base_new();
    ctx->base = base;
    cloud_add_timer_task(arg, nav_data_msg_task_cb, 1000);
    cloud_add_timer_task(arg, reboot_upgrade_task_cb, 30000);
    cloud_add_timer_task(arg, ota_heartbeat_task_cb, 60000);

    event_base_dispatch(base);  // 启动事件循环
    
    if (ctx->ev_list.count > 0) {
        while((pNode = List_GetHead(&ctx->ev_list)) != NULL) {
            event_free((struct event *)(pNode->arg));
            List_DelHead(&ctx->ev_list);
        }
    }
    event_base_free(base);

    return NULL;
}

void clound_comm_init(CloundCommContext *ctx)
{
    TcpClient *client = NULL;

    FX650_Error ret = fx650_init(&ctx->fx650);
    if (FX650_OK != ret) {
        spdlog_error("fx650_init failed. %d.", ret);
        return;
    }

    laneTo_init(&ctx->laneTo);
    init_queue(&ctx->event_queue, 256);
    List_Init_Thread(&ctx->ev_list);
    List_Init_Thread(&ctx->down_task.list);
    if (ctx->base_info == NULL) {
        client = tcp_client_create(CLOUD_SERVER_IP, CLOUD_SERVER_PORT, MAX_RECONNECT_ATTEMPTS);
    } else {
        client = tcp_client_create(ctx->base_info->cloud_ip, ctx->base_info->cloud_port, MAX_RECONNECT_ATTEMPTS);
    }
    client->ops->register_cb(client, proc_message_cb);
    client->ops->connect(client);
    ctx->client = client;
    ctx->running = true;
    pthread_create(&ctx->timer_thread, NULL, cloud_timer_task_entry, ctx);
    pthread_create(&ctx->event_thread, NULL, cloud_event_task_entry, ctx);
    if ((pthread_mutex_init(&ctx->down_task.mutex, NULL) == 0) && 
        (pthread_cond_init(&ctx->down_task.cond, NULL) ==0)) {
        pthread_create(&ctx->down_task.thread, NULL, download_upgrade_entry, ctx);
    }
    gp_cloud_comm_ctx = ctx;
    spdlog_info("cloud_comm init ok");
}

void clound_comm_uninit(CloundCommContext *ctx)
{

    if (ctx == NULL || ctx->running == false) {
        return ;
    } 

    spdlog_debug("clound_comm_uninit.");
    ctx->running = false;
    event_base_loopbreak(ctx->base);
    pthread_join(ctx->timer_thread, NULL);
    pthread_join(ctx->event_thread, NULL);
    pthread_join(ctx->down_task.thread, NULL);
    pthread_mutex_destroy(&ctx->down_task.mutex);
    pthread_cond_destroy(&ctx->down_task.cond);
    if (ctx->client->is_connected) {
        ctx->client->ops->disconnect(ctx->client);
    }
    tcp_client_destroy(ctx->client);
    laneTo_uninit(&ctx->laneTo);
    fx650_uninit(&ctx->fx650);
    clean_queue(&ctx->event_queue);
    gp_cloud_comm_ctx = NULL;
}

