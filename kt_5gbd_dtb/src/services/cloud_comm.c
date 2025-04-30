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

extern SGData sg_data;
extern struct FwUpdater __start_firmware_update;
extern struct FwUpdater __stop_firmware_update;

extern struct MsgProcInf __start_message_processing;
extern struct MsgProcInf __stop_message_processing;

static CloundCommContext *gp_cloud_comm_ctx = NULL;

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

static void GetFileName(const char *url, char *filename)
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
#if 0
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
    SSHClient_Init(&ssh_client, MQTT_SERVER_IP, MQTT_SERVER_USERNAME, MQTT_SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.connect failed.\n");
        return -1;
    }
#if 0
    char resp[128];
    ret = ssh_client.execute(&ssh_client, "psql -U cktt  -d fkz9 -t -c \"set search_path=obc; select dev_addr from device_info\" | awk -F ' ' '{print $1}'", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute psql failed.\n");
        return -1;
    }
    snprintf(pHb_info->dev_addr, sizeof(pHb_info->dev_addr), "%04d", resp);

    ret = ssh_client.execute(&ssh_client, "top -b -n 1 | grep \"%%Cpu(s)\" | awk '{print $2}' | awk -F. '{print $1}'",
             pHb_info->cpu_info, sizeof(pHb_info->cpu_info));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute cpu_info failed.\n");
        return -1;
    }

    ret = ssh_client.execute(&ssh_client, "$(df -B 1 | awk '/sda1/{print $2, $3}'",
             resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute disk failed.\n");
        return -1;
    }
    sscanf(resp, "%lld %lld", pHb_info->total_disk, pHb_info->used_disk);

    ret = ssh_client.execute(&ssh_client, "$(free -b | awk 'NR==2{print $2, $3}'",
             resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute memory failed.\n");
        return -1;
    }
    sscanf(resp, "%lld %lld", pHb_info->total_mem, pHb_info->used_mem);
    
    ret = ssh_client.execute(&ssh_client, "uptime -s",
             pHb_info->up_time, sizeof(pHb_info->up_time));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute uptime failed.\n");
        return -1;
    }

    ret = ssh_client.execute(&ssh_client, "date \"+%%Y-%%m-%%d %%H:%%M:%%S\"",
             pHb_info->cur_time, sizeof(pHb_info->cur_time));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute curtime failed.\n");
        return -1;
    }

    struct UnitCorrInfo unit_info_table[] = {
        { .name = "中央处理单元", .sw_str = "cpu_soft_ver", .hw_str = "cpu_hard_ver" },
        { .name = "采集单元",   .sw_str = "ad_soft_ver",    .hw_str = "ad_hard_ver" },
        { .name = "控制单元",   .sw_str = "control_soft_ver", .hw_str = "control_hard_ver" },
        { .name = "网络单元",   .sw_str = "net_soft_ver",   .hw_str = "net_hard_ver" }
    };
    char req[512];
    snprintf(req, sizeof(req), "psql -U cktt -d fkz9 -t -A -c \"set search_path=obc; select %s,%s,%s,%s,%s,%s,%s,%s from device_info\"",
                unit_info_table[0].sw_str, unit_info_table[0].hw_str, unit_info_table[1].sw_str, unit_info_table[1].hw_str,
                unit_info_table[2].sw_str, unit_info_table[2].hw_str, unit_info_table[3].sw_str, unit_info_table[3].hw_str);

    ret = ssh_client.execute(&ssh_client, req, resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute version_info failed.\n");
        return -1;
    }
    pHb_info->hw_unit_num = sizeof(unit_info_table) / sizeof(unit_info_table[0]);
    pHb_info->hw_unit = (UnitInfo *)malloc(sizeof(struct st_UnitInfo) * pHb_info->hw_unit_num);
    pHb_info->sw_unit_num = sizeof(unit_info_table) / sizeof(unit_info_table[0]);;
    pHb_info->sw_unit = (UnitInfo *)malloc(sizeof(struct st_UnitInfo) * pHb_info->hw_unit_num);
    sscanf(resp, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
            pHb_info->sw_unit[0].unit_ver, pHb_info->hw_unit[0].unit_ver,
            pHb_info->sw_unit[1].unit_ver, pHb_info->hw_unit[1].unit_ver,
            pHb_info->sw_unit[2].unit_ver, pHb_info->hw_unit[2].unit_ver,
            pHb_info->sw_unit[3].unit_ver, pHb_info->hw_unit[3].unit_ver);
    for (int i = 0, i < pHb_info->hw_unit_num; i++) {
        strcpy(pHb_info->sw_unit[i].unit_name, unit_info_table[i].name);
        strcpy(pHb_info->hw_unit[i].unit_name, unit_info_table[i].name);
    }
#endif

    char resp[256];
    ret = ssh_client.execute(&ssh_client, "bash /home/cktt/script/updater.sh base_info", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute updater.sh base_info failed.\n");
        return -1;
    }
    sscanf(resp, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",
            pHb_info->dev_addr, pHb_info->cpu_info, 
            pHb_info->total_disk, pHb_info->used_disk, 
            pHb_info->total_mem, pHb_info->used_mem,
            pHb_info->up_time, pHb_info->cur_time);
    
    ret = ssh_client.execute(&ssh_client, "bash /home/cktt/script/updater.sh unit_info", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute updater.sh unit_info failed.\n");
        return -1;
    }
    
    char *token = strtok((char *)resp, ";");
    while (token != NULL) {
        pHb_info->unit_num++;
        token = strtok(NULL, ";");
    }
    pHb_info->units = (UnitInfo *)malloc(sizeof(struct st_UnitInfo) * pHb_info->unit_num);
    
    int i = 0;
    token = strtok((char *)resp, ";");
    while (token != NULL) {
        sscanf(token, "%[^:]:%[^,],%s", pHb_info->units[i].unit_name,
                pHb_info->units[i].sw_ver, pHb_info->units[i].hw_ver);
        i++;
        token = strtok(NULL, ";");
    }
#endif
    SSHClient_Destroy(&ssh_client);

    return -1;
}

cJSON *create_unit_info_object(UnitInfo * unit_info, uint8_t type)
{
    cJSON *obj = NULL;
    char version[64];

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
 
    int ret = get_ota_heartbeat_info(&heart_beat);
    if (ret) {
        return -1;
    }
    root = cJSON_CreateObject(); 
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    // sprintf(str, "%hu", heart_beat.dev_addr);
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
    strncpy(data, buf, strlen(buf));
    cJSON_Delete(root);
    free(buf);

    return 0;
}

int get_ota_report_info(struct FwDownInfo *info, void *arg)
{
    SSHClient ssh_client;
    struct st_OtaReport *pReport = NULL;

    if (arg != NULL) {
        return -1;
    }

    pReport = (struct st_OtaReport*)arg;
    SSHClient_Init(&ssh_client, MQTT_SERVER_IP, MQTT_SERVER_USERNAME, MQTT_SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.connect failed.\n");
        return -1;
    }

    char resp[256];
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "bash /home/cktt/script/updater.sh download_info %2d %s %s %s", 
                info->id, info->url, info->md5, info->type);
    ret = ssh_client.execute(&ssh_client, cmd, resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.execute updater.sh report_info failed.\n");
        return -1;
    }

    sscanf(resp, "%[^,],%[^,],%[^,],%[^,],",
            pReport->dev_addr, pReport->task_id, pReport->time, pReport->report);
    SSHClient_Destroy(&ssh_client);

    return 0;
}

int create_ota_report_data(struct FwDownInfo *info, char *data)
{
    cJSON *root = NULL;
    OtaReport report;

    if (data == NULL) {
        return -1;
    }

    memset(&report, 0, sizeof(OtaReport));
    root = cJSON_CreateObject();
    get_ota_report_info(info, (void *)&report);
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    cJSON_AddStringToObject(root, "deviceAddress", report.dev_addr);
    cJSON_AddStringToObject(root, "taskId", report.task_id);
    cJSON_AddStringToObject(root, "executionTime", report.time);
    cJSON_AddStringToObject(root, "executionReport", report.report);
    data = cJSON_Print(root);
    cJSON_Delete(root);

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
        fprintf(stderr, "fw_up->trans_func failed.\n");
        return ret;
    }
    if (fw_up->update_func != NULL) {
        ret = fw_up->update_func((void *)pInfo);
        if (ret) {
            fprintf(stderr, "up->update_func failed.\n");
        } else {
            if (fw_up->update_cb != NULL) {
                fw_up->update_cb((void *)pInfo);
            }
        }
    }

    return ret;
}

// void ota_report_task_cb(evutil_socket_t fd, short event, void *arg)
void do_ota_report(struct FwDownInfo *info)
{
    char buf[512];
    char *resp = NULL;
    
    int ret = create_ota_report_data(info, buf);
    if (ret) {
        return ;
    }
    http_post_request(OTA_HEARTBEAT_URL, buf, &resp);
    printf("%s\n", resp);

    free(resp);
}


int do_downlaod_firmware(struct List *task_list)
{
    struct FwDownInfo *pInfo = NULL;
    struct ListNode *pNode = NULL;
    char file_name[20];
    char local_path[128];
    char cmd[64];
    struct FwUpdateInfo up_info;

    if (task_list == NULL) {
        return -1;
    }

    if (task_list->count > 0) {
        while((pNode = List_GetHead(task_list)) != NULL) {
            pInfo = (struct FwDownInfo *)pNode->arg;

            GetFileName(pInfo->url, file_name);
            snprintf(cmd, sizeof(cmd), "mkdir -p %s%d", UPGRADE_FILE_LOCAL_PATH, pInfo->id);
            _system_(cmd, NULL, 0);
            snprintf(local_path, sizeof(local_path), "%s%d/%s", UPGRADE_FILE_LOCAL_PATH, pInfo->id, file_name);
            int ret = ftp_download(pInfo->url, local_path, NULL, CLOUD_SERVER_USERNAME, CLOUD_SERVER_PASSWORD);
            if (!ret) {
                //通知去执行更新
                memset(&up_info, 0, sizeof(struct FwUpdateInfo));
                up_info.flag = 1;
                up_info.id = pInfo->id;
                strncpy(up_info.md5, pInfo->md5, sizeof(up_info.md5));
                strncpy(up_info.path, local_path, sizeof(up_info.path));
                strncpy(up_info.name, file_name, sizeof(up_info.name));
                ret = do_upgrade_firmware(&up_info);
                if (!ret) {
                    do_ota_report(pInfo);
                }
            }

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
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return -1;
    }

    // 提取 code 字段
    cJSON *code_obj = cJSON_GetObjectItemCaseSensitive(root, "code");
    if (cJSON_IsString(code_obj) && (code_obj->valuestring != NULL)) {
        const char *code = code_obj->valuestring;
        if (strcmp(code, "SUCCESS") == 0) {
            printf("send heartbeat success and receive respond\n");
        } else {
            printf("send heartbeat failed and respond %s %s\n", code, respond);
            return -1;
        }
    }

    // 提取 taskList 字段
    cJSON *task_list_obj = cJSON_GetObjectItemCaseSensitive(root, "taskList");
    if (cJSON_IsArray(task_list_obj)) {
        int length = cJSON_GetArraySize(task_list_obj);
        if (length == 0) {
            printf("heartbeat end\n");
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
                    List_Insert(task_list, (void*)pInfo);

                    printf("pInfo ID: %d, URL: %s, MD5: %s, Type: %s\n", pInfo->id, pInfo->url, pInfo->md5, pInfo->type);
                }
            }
        }
    }

    return 0;
}

void ota_heartbeat_task_cb(evutil_socket_t fd, short event, void *arg)
{
    CloundCommContext *ctx = NULL;
    char buf[1024];
    char *resp = NULL;

    int ret = create_ota_heartbeat_data(buf);
    if (ret) {
        return;
    }

    ret = http_post_request(OTA_HEARTBEAT_URL, buf, &resp);
    printf("resp to ota_heartbeat_req: %s, %d\n", resp, ret);

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

    free(resp);
}

void nav_data_msg_task_cb(evutil_socket_t fd, short event, void *arg) 
{
    MsgFramHdr *hdr = NULL;
    NAVDataSeg *nav_data = NULL;
    MsgDataFramCrc *crc = NULL;
    CustomTime t;
    uint8_t buf[512];
    char str[50];

    if (arg == NULL) {
        return ;
    }
    
    // RUN_LED_TOGGLE();
    CloundCommContext *ctx = (CloundCommContext *)arg;
    laneTo_read_nav_data(ctx->laneTo);
    memset(buf, 0, sizeof(buf));
    hdr = (MsgFramHdr *)buf;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MSG_SIGN_TRANS_NAV_DATA;
    uint16_t len = sizeof(MsgFramHdr) + sizeof(NAVDataSeg) + sizeof(MsgDataFramCrc);
    hdr->usLen = bswap_16(len);
    // printf("hdr->usLen %d, sizeof(NAVDataSeg) %ld\n", hdr->usLen, sizeof(NAVDataSeg));
    nav_data = (NAVDataSeg *)(buf + sizeof(MsgFramHdr));
    get_system_time(&t);
    TIME_TO_STR(&t, str);
    printf("time %s, sg_data.message_id %s, sg_data.latitude %.8lf\n", str, sg_data.message_id, sg_data.latitude);
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
    ctx = (CloundCommContext *)arg;
    pHdr = (MsgFramHdr *)buf;
    pHdr->usHdr = MSG_SIGN_WAVE_FILE_REQ;
    pHdr->ucSign = MSG_SIGN_WAVE_FILE_RESP;
    uint16_t len = sizeof(MsgFramHdr) + sizeof(WaveFileResp) + sizeof(MsgDataFramCrc);
    pHdr->usLen = bswap_16(len);    
    pResp = (WaveFileResp *)(buf + sizeof(MsgFramHdr));
    pResp->usDevAddr = bswap_16(CLIENT_DEV_ADDR);
    CustomTime t;
    get_system_time(&t);
    pResp->ucYear = t.usYear - 2000;
    pResp->ucMonth = t.ucMonth;
    pResp->ucDay = t.ucDay;
    pResp->ucHour = t.ucHour;
    pResp->ucMinute = t.ucMinute;
    pResp->ucCode = 0;

    pCrc = (MsgDataFramCrc *)(buf + sizeof(MsgFramHdr) + sizeof(WaveFileResp));
    pCrc->usCRC = checkSum_8((uint8_t *)pHdr, len - sizeof(MsgDataFramCrc));
    pCrc->usCRC = bswap_16(pCrc->usCRC);

    client = ctx->client;
    client->ops->send(client, buf, len);

    char remote_path[128];
    char local_path[128];
    uint16_t dev_addr = CLIENT_DEV_ADDR;
    snprintf(remote_path, sizeof(remote_path), "/%4d/wavefile/%4d%2d/%2d/%2d/%4d-wavedat-%4d%2d%2d%2d%2d.dat.gz", dev_addr, t.usYear, pResp->ucMonth, pResp->ucDay, 
                pResp->ucHour, dev_addr, t.usYear, pResp->ucMonth, pResp->ucDay, pResp->ucHour, pResp->ucMinute);
    int ret = ftp_upload(FTP_SERVER_URL, local_path, remote_path, CLOUD_SERVER_USERNAME, CLOUD_SERVER_PASSWORD);
    if (ret != 0) {
        return -1;
    }

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -f %s", local_path);
    _system_(cmd, NULL, 0);

    return 0;
}

int func_wave_file_req(void *arg)
{
    // MsgFramHdr *pHdr = NULL;
    WaveFileReq *pReq = NULL;
    //MsgDataFramCrc *pCrc = NULL;
    SSHClient ssh_client;

    if (arg == NULL) {
        return -1;
    }
    
    printf("func_wave_file_req\n");
    pReq = (WaveFileReq *)((uint8_t *)arg + sizeof(MsgFramHdr));
    char r_folder[128];
    uint16_t year = pReq->ucYear + 2000; 
    snprintf(r_folder, sizeof(r_folder), "%s%4d%2d%2d/%4d%2d%2d%2d/", WAVE_FILE_REMOTE_PATH, 
                year, pReq->ucMonth, pReq->ucDay, 
                year, pReq->ucMonth, pReq->ucDay, pReq->ucHour);
    char file_name[32];
    snprintf(file_name, sizeof(file_name), "%4d-wavedat-%4d%2d%2d%2d.dat.gz", pReq->usDevAddr, year, pReq->ucMonth, pReq->ucDay, pReq->ucHour);
    SSHClient_Init(&ssh_client, MQTT_SERVER_IP, MQTT_SERVER_USERNAME, MQTT_SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.connect failed.\n");
        return -1;
    }

    char l_folder[128];
    snprintf(l_folder, sizeof(l_folder), "%s%4d%2d%2d%2d/", WAVE_FILE_LOCAL_PATH,
                year, pReq->ucMonth, pReq->ucDay, pReq->ucHour);
    char r_path[256];
    snprintf(r_path, sizeof(r_path), "%s%s", r_folder, file_name);
    ret = ssh_client.download_file(&ssh_client, r_path, l_folder);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        fprintf(stderr, "ssh_client.download_file cp_wave_file_from_fkz9 failed.\n");
        return -1;
    }

    SSHClient_Destroy(&ssh_client);
    return 0;
}

REGISTER_MESSAGE_PROCESSING_INTERFACE(wave_file, MSG_SIGN_WAVE_FILE_REQ, func_wave_file_req, func_wave_file_resp);

void proc_message_cb(char *buf, size_t len)
{
    if (buf == NULL || len == 0) {
        return ;
    }

    printf("proc_message_cb %ld\n", len);
    enqueue(&gp_cloud_comm_ctx->event_queue, (uint8_t *)buf, len);
}


void *event_task_entry(void *arg)
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
        printf("pHdr->usHdr 0x%x, pCrc->usCRC 0x%x, crc 0x%x\n",bswap_16(pHdr->usHdr), bswap_16(pCrc->usCRC), crc);
        if (pHdr->usHdr != MSG_DATA_FRAM_HDR || crc != bswap_16(pCrc->usCRC)) {
            continue ;
        }
        printf("pHdr->ucSign 0x%x start %ln stop %ln\n", pHdr->ucSign, (uint64_t*)(start), (uint64_t*)end);
        for (; start != end; start++) {
            printf("start->sign 0x%x\n", start->sign);
            if (start->sign == pHdr->ucSign) {
                int ret = start->pFuncEntry(buf);
                if (ret == 0 && start->pFuncCb != NULL) {
                    start->pFuncCb(arg);
                }
            }
        }
    }
    
    return NULL;
}

void add_timer_task(void *arg, void (task_cb)(evutil_socket_t, short, void*), uint32_t ms)
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

void *timer_task_entry(void *arg)
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
    add_timer_task(arg, nav_data_msg_task_cb, 1000);
    add_timer_task(arg, ota_heartbeat_task_cb, 60000);

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

    ctx->fx650 = (Fx650Ctx *)malloc(sizeof(Fx650Ctx));
    FX650_Error ret = fx650_init(ctx->fx650);
    if (FX650_OK != ret) {
        printf("fx650_init failed. %d\n", ret);
        return;
    }
    ctx->laneTo = (LaneToCtx*)malloc(sizeof(LaneToCtx));
    laneTo_init(ctx->laneTo);

    init_queue(&ctx->event_queue, 256);
    List_Init_Thread(&ctx->ev_list);
    List_Init_Thread(&ctx->down_task.list);

    client = tcp_client_create(CLOUD_SERVER_IP, CLOUD_SERVER_PORT, MAX_RECONNECT_ATTEMPTS);
    client->ops->register_cb(client, proc_message_cb);
    client->ops->connect(client);
    ctx->client = client;
    ctx->running = true;
    pthread_create(&ctx->timer_thread, NULL, timer_task_entry, ctx);
    pthread_create(&ctx->event_thread, NULL, event_task_entry, ctx);
    if ((pthread_mutex_init(&ctx->down_task.mutex, NULL) == 0) && 
        (pthread_cond_init(&ctx->down_task.cond, NULL) ==0)) {
        pthread_create(&ctx->down_task.thread, NULL, download_upgrade_entry, ctx);
    }
    gp_cloud_comm_ctx = ctx;
}

void clound_comm_uninit(CloundCommContext *ctx)
{
    if (ctx == NULL || ctx->running == true) {
        return ;
    } 

    printf("clound_comm_uninit\n");
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
    laneTo_uninit(ctx->laneTo);
    fx650_uninit(ctx->fx650);
    clean_queue(&ctx->event_queue);
}

