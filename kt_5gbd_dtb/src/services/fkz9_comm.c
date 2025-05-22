#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <byteswap.h>
#include <event2/event.h>
#include "publib.h"
#include "queue.h"
#include "list.h"
#include "mqtt_client.h"
#include "ftp_handler.h"
#include "cloud_comm.h"
#include "ssh_client.h"
#include "fkz9_comm.h"
#include "firmware_updater.h"
#include "spdlog_c.h"


static void heartbeat_resp(uint8_t *arg)
{
    HeartBeatDataSeg *hb_data = NULL;

    if (arg == NULL) {
        return;
    }

    hb_data = (HeartBeatDataSeg*)((uint8_t*)arg + sizeof(MsgFramHdr));
    spdlog_info("heartbeat_resp, DevAddr %04d, %02d-%02d-%2d %2d:%2d:%2d.", hb_data->usDevAddr, 
        hb_data->ucYear, hb_data->ucMonth, hb_data->ucDay, hb_data->ucHour, hb_data->ucMinute, hb_data->ucSecond);

    return;
}

static void on_message_cb(const char* topic, const void* payload, size_t payload_len)
{
    MsgFramHdr *pHdr = NULL;
    MsgDataFramCrc *pCrc = NULL;
    
    if (topic == NULL && payload == NULL && payload_len == 0) {
        return;
    }

    pHdr = (MsgFramHdr *)payload;
    uint16_t crc = checkSum_8((uint8_t *)payload, bswap_16(pHdr->usLen) - sizeof(MsgDataFramCrc));
    pCrc = (MsgDataFramCrc *)(payload + bswap_16(pHdr->usLen) - sizeof(MsgDataFramCrc));

    if (pHdr->usHdr != MSG_DATA_FRAM_HDR || crc != bswap_16(pCrc->usCRC)) {
        return ;
    }
    switch (pHdr->ucSign) {
        case MQTT_MSG_SIGN_HEARTBEAT_RESP:
            heartbeat_resp((uint8_t *)payload);
            break;
        default:
        break;
    }
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
    hdr = (MsgFramHdr*)buf;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MQTT_MSG_SIGN_HEARTBEAT_REQ;
    uint16_t len = sizeof(MsgFramHdr) + sizeof(HeartBeatDataSeg) + sizeof(MsgDataFramCrc);
    hdr->usLen = bswap_16(len);
    hb_data = (HeartBeatDataSeg*)(hdr + 1);
    hb_data->usDevAddr = bswap_16(CLIENT_DEV_ADDR);
    CustomTime t;
    get_system_time(&t);
    hb_data->ucYear = t.usYear - 2000;
    hb_data->ucMonth = t.ucMonth;
    hb_data->ucDay = t.ucDay;
    hb_data->ucHour = t.ucHour;
    hb_data->ucMinute = t.ucHour;
    hb_data->ucSecond = t.ucSecond;
    crc = (MsgDataFramCrc*)(hb_data + 1);
    crc->usCRC = checkSum_8((uint8_t*)hdr, len - sizeof(MsgDataFramCrc));
    crc->usCRC = bswap_16(crc->usCRC);
    // printf("MQTTAsync_setCallbacks sign=0x%x, crc=0x%x\n", hdr->ucSign, crc->usCRC);
    mqtt_client = ctx->mqtt_client;
    char topic[50] = {0};
    snprintf(topic, sizeof(topic), "fkz9/%d%s", CLIENT_DEV_ADDR, MQTT_HEARTBEAT_REQ_TOPIC);
    mqtt_client->ops->publish(mqtt_client, topic, buf, len);

    return;
}

static void add_timer_task(void *arg, void (task_cb)(evutil_socket_t, short, void*), uint32_t ms)
{
    Fkz9CommContext *ctx = NULL;
    
    if (arg == NULL || task_cb == NULL) {
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
            event_free((struct event *)(pNode->arg));
            List_DelHead(&ctx->ev_list);
        }
    }
    event_base_free(base);

    return NULL;
}

int init_updater_environment(void)
{
    SSHClient ssh_client;

    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.connect failed.");
        return -1;
    }

    char resp[128];
    ret = ssh_client.execute(&ssh_client, "find /home/cktt/script/ -name \"updater.sh\"", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute find updater.sh failed.");
        return -1;
    } else {
        spdlog_info("find updater.sh resp %s.", resp);
        if (strstr(resp, "updater.sh")) {
            SSHClient_Destroy(&ssh_client);
            return 0;
        }
    }
    
    char loacl_path[256];
    _system_("pwd", resp, sizeof(resp));
    snprintf(loacl_path, sizeof(loacl_path), "%s/kt_5gbd_dtb/script/updater.sh", resp);
    ret = ssh_client.upload_file(&ssh_client, loacl_path, "/home/cktt/script/updater.sh");
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.upload_file updater.sh failed.");
        return -1;
    }

    ret = ssh_client.execute(&ssh_client, "chmod +x /home/cktt/script/updater.sh", 
            resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute chmod +x updater.sh failed.");
        return -1;
    }

    SSHClient_Destroy(&ssh_client);
    return 0;
}

void fkz9_comm_init(Fkz9CommContext *ctx)
{
    AsyncMQTTClient *mqtt_client = NULL;
    char url[50] = {0};

    if (ctx == NULL) {
        return;
    }

    snprintf(url, sizeof(url), "tcp://%s:%d", MQTT_SERVER_IP, MQTT_SERVER_PORT);
    AsyncClientConfig client_config = {
        .address = url,
        .client_id = MQTT_CLIENT_ID,
        .user_name = MQTT_SERVER_USERNAME,
        .password = MQTT_SERVER_PASSWORD,
        .keep_alive = KEEP_ALIVE_TIME,
        .qos = QOS,
        .clean_session = 1
    };

    mqtt_client = mqtt_client_create(&client_config);
    if (mqtt_client == NULL) {
        return;
    }
    mqtt_client->ops->register_cb(mqtt_client, on_message_cb);
    int ret = mqtt_client->ops->connect(mqtt_client);
    if(ret) {
        spdlog_error("mqtt connect failed.");
        ctx->is_running = false;
        mqtt_client_destroy(mqtt_client);
        return;
    }
    init_updater_environment();
    ctx->mqtt_client = mqtt_client;
    init_queue(&ctx->tx_queue, MAX_MSG_SIZE);
    List_Init_Thread(&ctx->ev_list);
    pthread_create(&ctx->timer_thread, NULL, timer_task_entry, ctx);
    ctx->is_running = true;
}

void fkz9_comm_uninit(Fkz9CommContext *ctx)
{
    AsyncMQTTClient *mqtt_client = NULL;

    spdlog_debug("fkz9_comm_uninit.");
    if (ctx == NULL || ctx->is_running == false) {
        return;
    }

    ctx->is_running = false;
    event_base_loopbreak(ctx->base);
    pthread_join(ctx->timer_thread, NULL);
    mqtt_client = ctx->mqtt_client;
    mqtt_client_destroy(mqtt_client);
    clean_queue(&ctx->tx_queue);
}

