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
#include "tcp_client.h"
#include "cloud_comm.h"
#include "ssh_client.h"
#include "fkz9_comm.h"
#include "firmware_updater.h"
#include "spdlog_c.h"


//extern struct Fkz9MsgFwInf __start_message_forwarding;
//extern struct Fkz9MsgFwInf __stop_message_forwarding;
static Fkz9CommContext *gp_fkz9_comm_ctx = NULL;

static void func_dev_info_resp(void)
{
    MsgFramHdr *pHdr = NULL;
    DevInfoDataSeg *pInfo = NULL;
    MsgDataFramCrc *pCrc = NULL;
    uint8_t buf[128] = {0};

    pHdr = (MsgFramHdr *)buf;
    pHdr->usHdr = MSG_DATA_FRAM_HDR1;
    pHdr->ucSign = TCP_MSG_SIGN_DEV_INFO_RESP;
    uint16_t len = sizeof(MsgFramHdr) + sizeof(DevInfoDataSeg) + sizeof(MsgDataFramCrc);
    pHdr->usLen = bswap_16(len);

    pInfo = (DevInfoDataSeg *)(buf + sizeof(MsgFramHdr));
    pInfo->usDevAddr = bswap_16(fkz9_devBaseInfo.dev_addr);
    pInfo->ucSwVerCPU = fkz9_devBaseInfo.cpu_sw;
    pInfo->ucHwVerCPU = fkz9_devBaseInfo.cpu_hw;
    strcpy(pInfo->cSimID, gp_cloud_comm_ctx->fx650->sim_id);
    pInfo->ucSwVerCTU = fkz9_devBaseInfo.ctrl_sw;
    pInfo->ucHwVerCTU = fkz9_devBaseInfo.ctrl_hw;
    pInfo->ucSwVerAU = fkz9_devBaseInfo.au_sw;
    pInfo->ucHwVerAU = fkz9_devBaseInfo.au_hw;
    pInfo->ucSwVerNTU = fkz9_devBaseInfo.ntu_sw;
    pInfo->ucHwVerNTU = fkz9_devBaseInfo.ntu_hw;

    pCrc = (MsgDataFramCrc *)(buf + sizeof(MsgFramHdr) + sizeof(DevInfoDataSeg));
    pCrc->usCRC = checkSum_8(buf, len - sizeof(MsgDataFramCrc));
    pCrc->usCRC = bswap_16(pCrc->usCRC);

    TcpClient *client = gp_cloud_comm_ctx->client;
    client->ops->send(client, buf, len);
}


static int fkz9_heartbeat_resp_entry(void *arg)
{
    HeartBeatDataSeg *hb_data = NULL;

    if (arg == NULL) {
        return -1;
    }

    hb_data = (HeartBeatDataSeg*)((uint8_t*)arg + sizeof(MsgFramHdr));
    spdlog_info("heartbeat_resp, DevAddr %04d, %02d-%02d-%2d %2d:%2d:%2d.", hb_data->usDevAddr, 
        hb_data->ucYear, hb_data->ucMonth, hb_data->ucDay, hb_data->ucHour, hb_data->ucMinute, hb_data->ucSecond);

    if (gp_fkz9_comm_ctx->is_init == false) {
        func_dev_info_resp();
        gp_fkz9_comm_ctx->is_init = true;
    }
    
    return 0;
}
//REGISTER_FKZ9_MESSAGE_FW_INTERFACE(hb_resp, 22, fkz9_heartbeat_resp_entry, NULL);

extern CloundCommContext *gp_cloud_comm_ctx;
static int fkz9_msg_fw_to_cloud_entry(void *arg)
{
    MsgFramHdr *pHdr = NULL;

    if (arg == NULL) {
        return -1;
    }

    pHdr = (MsgFramHdr*)arg;
    TcpClient *client = gp_cloud_comm_ctx->client;
    client->ops->send(client, (uint8_t*)arg, bswap_16(pHdr->usLen));

    return 0;
}

static int cloud_to_can_entry(void* arg)
{
    MsgFramHdr *pHdr = NULL;
    AsyncMQTTClient *mqtt_client = NULL;

    if (arg == NULL) {
        return -1;
    }

    pHdr = (MsgFramHdr*)arg;
    mqtt_client = gp_fkz9_comm_ctx->mqtt_client;
    char topic[50] = {0};
    snprintf(topic, sizeof(topic), "fkz9/%04d/4G/can/%02x", gp_fkz9_comm_ctx->fkz9_dev_addr, pHdr->ucSign);
    mqtt_client->ops->publish(mqtt_client, topic, arg, bswap_16(pHdr->usLen));
    
    return 0;
}

static int cloud_to_conf_entry(void* arg)
{
    MsgFramHdr *pHdr = NULL;
    AsyncMQTTClient *mqtt_client = NULL;

    if (arg == NULL) {
        return -1;
    }

    pHdr = (MsgFramHdr*)arg;
    mqtt_client = gp_fkz9_comm_ctx->mqtt_client;
    char topic[50] = {0};
    snprintf(topic, sizeof(topic), "fkz9/%04d/4G/conf/%02x", gp_fkz9_comm_ctx->fkz9_dev_addr, pHdr->ucSign);
    mqtt_client->ops->publish(mqtt_client, topic, arg, bswap_16(pHdr->usLen));
    
    return 0;
}

static int cloud_to_algorithms_entry(void* arg)
{
    MsgFramHdr *pHdr = NULL;
    AsyncMQTTClient *mqtt_client = NULL;

    if (arg == NULL) {
        return -1;
    }

    pHdr = (MsgFramHdr*)arg;
    mqtt_client = gp_fkz9_comm_ctx->mqtt_client;
    char topic[50] = {0};
    snprintf(topic, sizeof(topic), "fkz9/%04d/4G/algorithms/%02x", gp_fkz9_comm_ctx->fkz9_dev_addr, pHdr->ucSign);
    mqtt_client->ops->publish(mqtt_client, topic, arg, bswap_16(pHdr->usLen));
    
    return 0;
}

static int cloud_to_cmd_entry(void* arg)
{
    MsgFramHdr *pHdr = NULL;
    AsyncMQTTClient *mqtt_client = NULL;

    if (arg == NULL) {
        return -1;
    }

    pHdr = (MsgFramHdr*)arg;
    mqtt_client = gp_fkz9_comm_ctx->mqtt_client;
    char topic[50] = {0};
    snprintf(topic, sizeof(topic), "fkz9/%04d/4G/cmd/%02x", gp_fkz9_comm_ctx->fkz9_dev_addr, pHdr->ucSign);
    mqtt_client->ops->publish(mqtt_client, topic, arg, bswap_16(pHdr->usLen));
    
    return 0;
}
/* 0x91 服务器发送换参数据 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(change_refs, 145, cloud_to_can_entry, NULL);
/* 0x72 服务器发送录波开关数据 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(wave_switch, 114, cloud_to_conf_entry, NULL);
/* 0x59 服务器下行剩磁系数数据 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(remanence_coeff, 89, cloud_to_conf_entry, NULL);
/* 0x82 服务器下行突发录波参数配置 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(wave_record, 130, cloud_to_conf_entry, NULL);
/* 0x84 服务器下行首次合闸参数配置1 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(first_close1, 132, cloud_to_can_entry, NULL);
/* 0x87 服务器下行首次合闸参数配置2 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(first_close2, 135, cloud_to_can_entry, NULL);
// /* 0xC3 服务器下行时间预估算法参数 */
// REGISTER_MESSAGE_PROCESSING_INTERFACE(time_estimation, 195, cloud_to_can_entry, NULL);
// /* 0x08 服务器下行时间预估算法参数 */
// REGISTER_MESSAGE_PROCESSING_INTERFACE(feedback_time, 8, cloud_to_can_entry, NULL);
/* 0x93 服务器下行网压变比系数配置 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(voltage_ratio, 147, cloud_to_conf_entry, NULL);
/* 0x9C 服务器下行断路器预估算法配置 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(cb_pred_alg, 156, cloud_to_can_entry, NULL);
/* 0x9E 服务器下行相控功能配置 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(phase_control, 158, cloud_to_can_entry, NULL);
/* 0x6C 服务器新下行合闸相位预估算法开关控制 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(close_phase, 108, cloud_to_algorithms_entry, NULL);
/* 0x6E 服务器下行断路器二次侧关合时间开关控制 */
REGISTER_MESSAGE_PROCESSING_INTERFACE(cb_close_time, 110, cloud_to_algorithms_entry, NULL);

static void on_message_cb(const char* topic, const void* payload, size_t payload_len)
{
    // MsgFramHdr *pHdr = NULL;
    // MsgDataFramCrc *pCrc = NULL;
    
    if (topic == NULL && payload == NULL && payload_len == 0) {
        return;
    }

    spdlog_info("topic: %s, payload_len: %ld.", topic, payload_len);
    // pHdr = (MsgFramHdr *)payload;
    // uint16_t crc = checkSum_8((uint8_t *)payload, bswap_16(pHdr->usLen) - sizeof(MsgDataFramCrc));
    // pCrc = (MsgDataFramCrc *)((uint8_t *)payload + (bswap_16(pHdr->usLen) - sizeof(MsgDataFramCrc)));

    // if (pHdr->usHdr != MSG_DATA_FRAM_HDR1 || crc != bswap_16(pCrc->usCRC)) {
    //     return ;
    // }
    // switch (pHdr->ucSign) {
    //     case MQTT_MSG_SIGN_HEARTBEAT_RESP:
    //         heartbeat_resp((uint8_t *)payload);
    //         break;
    //     default:
    //     break;
    // }
    enqueue(&gp_fkz9_comm_ctx->event_queue, (uint8_t *)payload, payload_len);
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
    hdr->usHdr = MSG_DATA_FRAM_HDR1;
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
    snprintf(topic, sizeof(topic), "fkz9/%04d%s", ctx->fkz9_dev_addr, MQTT_HEARTBEAT_REQ_TOPIC);
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
    AsyncMQTTClient *mqtt_client = NULL;
    
    if (arg == NULL) {
        return NULL;
    }

    ctx = (Fkz9CommContext *)arg;
    base = event_base_new();
    ctx->base = base;
    add_timer_task(arg, heartbeat_req_task_cb, 3000);

    spdlog_info("fkz9_dev_addr=%d, ctx=0x%x", ctx->fkz9_dev_addr, ctx);
    char topic[128] = {0};
    snprintf(topic, sizeof(topic), "fkz9/%04d/+/4G/#", ctx->fkz9_dev_addr);
    mqtt_client = ctx->mqtt_client;
    do {
        sleep(0.1);
    }
    while (mqtt_client->is_conn == false);
    mqtt_client->ops->subscribe(mqtt_client, topic);

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

static void *event_task_entry(void *arg)
{
    Fkz9CommContext *ctx = NULL;
    uint8_t buf[1024];
    size_t len = 0;
    MsgFramHdr *pHdr = NULL;
    MsgDataFramCrc *pCrc = NULL;
    // struct Fkz9MsgFwInf *start = &__start_message_forwarding;
    // struct Fkz9MsgFwInf *end = &__stop_message_forwarding;

    if (arg == NULL) {
        return NULL;
    }
    
    ctx = (Fkz9CommContext *)arg;
    while (ctx->is_running) {
        if (dequeue(&ctx->event_queue, buf, &len)) {
            continue;
        }
        pHdr = (MsgFramHdr *)buf;
        uint16_t crc = checkSum_8((uint8_t *)buf, bswap_16(pHdr->usLen) - sizeof(MsgDataFramCrc));
        pCrc = (MsgDataFramCrc *)(buf + bswap_16(pHdr->usLen) - sizeof(MsgDataFramCrc));
        spdlog_debug("fkz9_recv: pHdr->usHdr=0x%x, pHdr->ucSign=0x%x, pCrc->usCRC=0x%x, crc=0x%x.", bswap_16(pHdr->usHdr), pHdr->ucSign, bswap_16(pCrc->usCRC), crc);
        if ((pHdr->usHdr != MSG_DATA_FRAM_HDR1 && pHdr->usHdr != MSG_DATA_FRAM_HDR2) || crc != bswap_16(pCrc->usCRC)) {
            continue ;
        }

        if (MQTT_MSG_SIGN_HEARTBEAT_RESP == pHdr->ucSign) {
            fkz9_heartbeat_resp_entry(buf);
        } else {
            fkz9_msg_fw_to_cloud_entry(buf);
        }
        
        // for (; start != end; start++) {
        //     spdlog_info("start->sign=0x%x, pHdr->ucSign", start->sign, pHdr->ucSign);
        //     if (start->sign == pHdr->ucSign) {
        //         int ret = start->pFuncEntry(buf);
        //         if (ret == 0 && start->pFuncCb != NULL) {
        //             start->pFuncCb(arg);
        //         }
        //     } else {
        //         fkz9_msg_fw_to_cloud_entry(buf);
        //     }
        // }
    }

    return NULL;
}

static int get_fkz9_dev_addr(uint16_t* dev_addr)
{
#if 0
    SSHClient ssh_client;

    SSHClient_Init(&ssh_client, SERVER_IP, SERVER_USERNAME, SERVER_PASSWORD);
    int ret = ssh_client.connect(&ssh_client);
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.connect failed.");
        return -1;
    }

    char resp[128] = {0};
    ret = ssh_client.execute(&ssh_client, "bash /home/cktt/script/updater.sh base_info", resp, sizeof(resp));
    if (ret) {
        SSHClient_Destroy(&ssh_client);
        spdlog_error("ssh_client.execute updater.sh failed.");
        return -1;
    }
    sscanf(resp, "%hd,", dev_addr);

    SSHClient_Destroy(&ssh_client);
#else
    *dev_addr = fkz9_devBaseInfo.dev_addr;
#endif
    return 0;
}

void fkz9_comm_init(Fkz9CommContext *ctx)
{
    AsyncMQTTClient *mqtt_client = NULL;
    char url[50] = {0};

    if (ctx == NULL) {
        return;
    }
    
    get_fkz9_dev_addr(&(ctx->fkz9_dev_addr));
    spdlog_info("kfz9_dev_addr=%d, ctx=0x%x", ctx->fkz9_dev_addr, ctx);
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
        return;
    }

    ctx->mqtt_client = mqtt_client;
    init_queue(&ctx->event_queue, MAX_MSG_SIZE);
    List_Init_Thread(&ctx->ev_list);
    pthread_create(&ctx->timer_thread, NULL, timer_task_entry, ctx);
    pthread_create(&ctx->event_thread, NULL, event_task_entry, ctx);
    ctx->is_running = true;
    ctx->is_init = false;
    gp_fkz9_comm_ctx = ctx;
}

void fkz9_comm_uninit(Fkz9CommContext *ctx)
{
    AsyncMQTTClient *mqtt_client = NULL;

    if (ctx == NULL || ctx->is_running == false) {
        return;
    }

    spdlog_debug("fkz9_comm_uninit.");
    ctx->is_running = false;
    ctx->is_init = false;
    event_base_loopbreak(ctx->base);
    pthread_join(ctx->timer_thread, NULL);
    pthread_join(ctx->event_thread, NULL);
    mqtt_client = ctx->mqtt_client;
    mqtt_client_destroy(mqtt_client);
    clean_queue(&ctx->event_queue);
    gp_fkz9_comm_ctx = NULL;
}

