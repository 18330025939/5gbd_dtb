#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <event2/event.h>
#include "publib.h"
#include "queue.h"
#include "list.h"
#include "mqtt_client.h"
#include "ftp_handler.h"
#include "cloud_comm.h"
#include "ssh_client.h"
#include "fkz9_comm.h"

static void heartbeat_resp(uint8_t *arg)
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

static void on_message_cb(const char* topic, const void* payload, size_t payload_len)
{
    MsgFramHdr *pHdr = NULL;
    MsgDataFramCrc *pCrc = NULL;
    
    if (topic == NULL && payload == NULL && payload_len == 0) {
        return;
    }

    pHdr = (MsgFramHdr *)payload;
    uint16_t crc = checkSum_8((uint8_t *)payload, pHdr->usLen);
    pCrc = (MsgDataFramCrc *)(payload + pHdr->usLen);

    if (pHdr->usHdr != MSG_DATA_FRAM_HDR || crc != pCrc->usCRC) {
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
    mqtt_client = ctx->mqtt_client;
    hdr = (MsgFramHdr*)buf;
    hdr->usHdr = MSG_DATA_FRAM_HDR;
    hdr->ucSign = MQTT_MSG_SIGN_HEARTBEAT_REQ;
    hdr->usLen = sizeof(MsgFramHdr) + sizeof(HeartBeatDataSeg);
    hb_data = (HeartBeatDataSeg*)(hdr + 1);
    hb_data->usDevAddr = 0;
    get_system_time(&(hb_data->stTime));
    crc = (MsgDataFramCrc*)(hb_data + 1);
    crc->usCRC = checkSum_8((uint8_t*)hdr, hdr->usLen);

    if (mqtt_client->is_conn) {
        char topic[50] = {0};
        snprintf(topic, sizeof(topic), "fkz9/%d%s", ctx->fkz9_dev_addr, MQTT_HEARTBEAT_REQ_TOPIC);
        mqtt_client->ops->publish(mqtt_client, topic, buf, hdr->usLen + sizeof(MsgDataFramCrc));
    }

    return;
}

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
            event_free((struct event *)(pNode->arg));
            List_DelHead(&ctx->ev_list);
        }
    }
    event_base_free(base);

    return NULL;
}

void fkz9_comm_init(Fkz9CommContext *ctx)
{
    AsyncMQTTClient *mqtt_client = NULL;
    char url[50] = {0};

    if (ctx == NULL) {
        return;
    }

    init_queue(&ctx->tx_queue, MAX_MSG_SIZE);
    List_Init_Thread(&ctx->ev_list);
    // init_queue(ctx->rx_queue, MAX_MSG_SIZE);
    snprintf(url, sizeof(url), "tcp://%s:%d", MQTT_SERVER_IP, MQTT_SERVER_PORT);
    mqtt_client = mqtt_client_create(url, MQTT_CLIENT_ID, MQTT_SERVER_USERNAME, MQTT_SERVER_PASSWORD);
    mqtt_client->ops->register_cb(mqtt_client, on_message_cb);
    int ret = mqtt_client->ops->connect(mqtt_client);
    if(ret) {
        printf("mqtt connect failed\n");
        ctx->is_running = false;
        clean_queue(&ctx->tx_queue);
        mqtt_client_destroy(mqtt_client);
        return;
    }
    ctx->mqtt_client = mqtt_client;
    pthread_create(&ctx->timer_thread, NULL, timer_task_entry, ctx);
    ctx->is_running = true;
}

void fkz9_comm_uninit(Fkz9CommContext *ctx)
{
    AsyncMQTTClient *mqtt_client = NULL;

    printf("fkz9_comm_uninit\n");
    if (ctx == NULL || ctx->is_running == false) {
        return;
    }

    event_base_loopbreak(ctx->base);
    pthread_join(ctx->timer_thread, NULL);
    mqtt_client = ctx->mqtt_client;
    mqtt_client_destroy(mqtt_client);
    clean_queue(&ctx->tx_queue);
}

