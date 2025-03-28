#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "cJSON.h"
#include "queue.h"
#include "tcp_client.h"
#include "ftp_handler.h"
#include "cloud_comm.h"



uint16_t checkSum_8(uint8_t *buf, uint16_t len) //buf为数组，len为数组长度
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


void init_post_request(PostRequest *post_req)
{
    if (post_req == NULL) {
        return;
    }

    post_req->dev_addr = strudp();
    post_req->usage_cpu = strdup();
    post_req->up_time = strdup();
    post_req->sys_time = strdup();


}

void uninit_post_request(PostRequest *post_req)
{
    size_t i = 0;

    if (post_req == NULL) {
        return;
    }

    free(post_req->dev_addr);
    free(post_req->usage_cpu);
    free(post_req->up_time);
    free(post_req->sys_time);
    if (post_req->hw_unit != NULL) {
        free(post_req->hw_unit);
    }

    if (post_req->sw_unit != NULL) {
        free(post_req->sw_unit);
    }
}


cJSON *create_unit_info_obj(UnitInfo * unit_info)
{
    cJSON *obj = NULL;

    obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, unit_info->unit.key, unit_info->unit.val);
    cJSON_AddStringToObject(obj, unit_info->version.key, unit_info->version.val);
    
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
    PostRequest post_req;
    char *buf = NULL;

    if (data == NULL) {
        return -1;
    }

    root = cJSON_CreateObject();
    init_post_request(&post_req);
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    cJSON_AddStringToObject(root, "deviceAddress", post_req.dev_addr);
    cJSON_AddStringToObject(root, "usageCpu", post_req.usage_cpu);
    cJSON_AddStringToObject(root, "usageMemory", post_req.usage_mem);
    cJSON_AddStringToObject(root, "totalMemory", post_req.total_mem);
    cJSON_AddStringToObject(root, "usageDisk", post_req.usage_disk);
    cJSON_AddStringToObject(root, "totalDisk", post_req.total_disk);
    cJSON_AddStringToObject(root, "upTime", post_req.up_time);
    cJSON_AddStringToObject(root, "systemTime", post_req.sys_time);
    cJSON_AddStringToObject(root, "extendInfo", "");
    unit_info = create_unit_info_array(post_req.hw_unit_num, post_req.hw_unit);
    cJSON_AddItemToObject(root, "hardwareList", unit_info);
    unit_info = create_unit_info_array(post_req.sw_unit_num, post_req.sw_unit);
    cJSON_AddItemToObject(root, "softwareList", unit_info);
    uninit_post_request(&post_req);

    buf = cJSON_Print(root);
    strncpy(data, buf, strlen(buf));
    cJSON_Delete(root);

    return 0;
}
// http_post_request

int create_ota_report_data(char *data)
{

    cJSON *root = NULL;

    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "lang", "zh_CN");
    cJSON_AddStringToObject(root, "deviceAddress", );
    cJSON_AddStringToObject(root, "taskId", );
    cJSON_AddStringToObject(root, "executionTime", );
    cJSON_AddStringToObject(root, "executionReport", );
    data = cJSON_Print(root);
    cJSON_Delete(root);

}


int create_ota_update_data(char *data)
{



}

