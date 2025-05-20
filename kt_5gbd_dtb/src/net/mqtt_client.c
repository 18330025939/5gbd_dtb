#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "mqtt_client.h"
#include "spdlog_c.h"


/* 连接成功回调 */
static void on_connect_success(void* context, MQTTAsync_successData* response)
{
    AsyncMQTTClient* client = (AsyncMQTTClient*)context;
    pthread_mutex_lock(&client->lock);
    client->is_conn = true;
    // if (client->on_connected) {
    //     client->on_connected(client->user_ctx, 0, "Connected successfully");
    // }
    pthread_mutex_unlock(&client->lock);
    
    spdlog_debug("MQTT service connection successful.");
}

/* 连接失败回调 */
static void on_connect_failure(void* context, MQTTAsync_failureData* response) 
{
    AsyncMQTTClient* client = (AsyncMQTTClient*)context;
    pthread_mutex_lock(&client->lock);
    if (client->is_conn) {
        client->is_conn = false;
    }
    // if (client->on_connected) {
    //     const char* msg = response ? response->message : "Unknown error";
    //     client->on_connected(client->user_ctx, response->code, msg);
    // }
    pthread_mutex_unlock(&client->lock);
    spdlog_error("MQTT service connection failed, rc %d.", response ? response->code : 0);
}

/* 消息到达回调 */
static int message_arrived_cb(void* context, char* topic, int topic_len, MQTTAsync_message* msg) 
{
    AsyncMQTTClient* client = (AsyncMQTTClient*)context;
    pthread_mutex_lock(&client->lock);
    if (client->on_message) {
        client->on_message(topic, msg->payload, msg->payloadlen);
    }
    MQTTAsync_freeMessage(&msg);
    MQTTAsync_free(topic);
    pthread_mutex_unlock(&client->lock);
    return 1;
}

/* 连接管理 */
int mqtt_connect(AsyncMQTTClient* client) 
{
    if (client == NULL) {
        return -1;
    }

    AsyncClientConfig *config = client->config;
    
    // 配置连接参数
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = config->keep_alive;
    conn_opts.cleansession = config->clean_session;
    conn_opts.username = config->user_name;
    conn_opts.password = config->password;
    conn_opts.context = client;
    conn_opts.onSuccess = on_connect_success;
    conn_opts.onFailure = on_connect_failure;
    
    pthread_mutex_lock(&client->lock);
    int rc = MQTTAsync_connect(client->handle, &conn_opts);
    pthread_mutex_unlock(&client->lock);

    spdlog_debug("mqtt_connect username=%s, password=%s, rc=%d.", conn_opts.username, conn_opts.password, rc);
    return rc;
}

void on_publish_success(void* context, MQTTAsync_successData* response)
{
    spdlog_debug("publish success.");
}

/* 消息发布 */
int mqtt_publish(AsyncMQTTClient* client, const char* topic, const void* payload, size_t len) 
{
    AsyncClientConfig *config = client->config;
    int rc = -1;
    
    MQTTAsync_message msg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
    msg.payload = (void*)payload;
    msg.payloadlen = len;
    msg.qos = config->qos;
    msg.retained = 0;
    pub_opts.onSuccess = on_publish_success;
    pub_opts.context = client;

    pthread_mutex_lock(&client->lock);
    if (client->is_conn == true) {
        rc = MQTTAsync_sendMessage(client->handle, topic, &msg, &pub_opts);
    }
    pthread_mutex_unlock(&client->lock);

    spdlog_debug("mqtt_publish topic=%s, len=%ld, rc=%d.", topic, len, rc);
    char *str = (char *)malloc(len * 4 + 1);
    if (str != NULL) {
        str[0] = '\0';
        for (int i = 0; i < len; i++) {
            sprintf(str + strlen(str), "%hhu ", ((uint8_t*)payload)[i]);
        }

        if (strlen(str) > 0) {
            str[strlen(str) - 1] = '\0';
        }

        spdlog_debug("payload: %s", str);
        free(str);
    }

    return rc;
}

static void on_subscribe_success(void* context, MQTTAsync_successData* response)
{
	spdlog_debug("Subscribe succeeded.");
}

static void on_subscribe_failure(void* context, MQTTAsync_failureData* response)
{
	spdlog_error("Subscribe failed, rc %d.", response->code);
}

/* 消息订阅 */
int mqtt_subscribe(AsyncMQTTClient* client, const char* topic) 
{
    AsyncClientConfig *config = client->config;
    int rc = -1;
    
    MQTTAsync_responseOptions sub_opts = MQTTAsync_responseOptions_initializer;
    sub_opts.onSuccess = on_subscribe_success;
	sub_opts.onFailure = on_subscribe_failure;
	sub_opts.context = client;

    spdlog_debug("mqtt_subscribe topic=%s.", topic);
    pthread_mutex_lock(&client->lock);
    if (client->is_conn == true) {
        rc = MQTTAsync_subscribe(client->handle, topic, config->qos, &sub_opts);
    }
    pthread_mutex_unlock(&client->lock);
    return rc;
}

static void mqtt_register_cb(AsyncMQTTClient* client, void (*cb)(const char*, const void*, size_t))
{
    client->on_message = cb;
}

static AsyncClientOps client_ops = {
    .connect = mqtt_connect,
    .publish = mqtt_publish,
    .subscribe = mqtt_subscribe,
    .register_cb = mqtt_register_cb
} ;

// static AsyncClientConfig client_config = {
//     .keep_alive = KEEP_ALIVE_TIME,
//     .qos = QOS,
//     .clean_session = 1,
// } ;

/* 初始化 */
AsyncMQTTClient* mqtt_client_create(AsyncClientConfig *config)
{
    int rc;
    AsyncMQTTClient* client = calloc(1, sizeof(AsyncMQTTClient));
    pthread_mutex_init(&client->lock, NULL);

    client->config = config;

    client->ops = &client_ops;
    client->is_conn = false;

    spdlog_debug("mqtt_client_create addr=%s, id=%s, username=%s, password=%s.", client->config->address,
                   client->config->client_id, client->config->user_name, client->config->password);
    if ((rc = MQTTAsync_create(&client->handle, client->config->address, client->config->client_id, 
                        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS) {
        spdlog_error("Failed to create client object, return code %d.", rc);
        goto err_exit;
    }

    if ((rc = MQTTAsync_setCallbacks(client->handle, client, NULL, message_arrived_cb,  
                        NULL)) != MQTTASYNC_SUCCESS) {
        spdlog_error("Failed to set callback, return code %d.", rc);
        goto err_exit;                    
    }
    return client;

err_exit:
    mqtt_client_destroy(client);
    return NULL;
}

/* 销毁 */
void mqtt_client_destroy(AsyncMQTTClient* client) 
{
    if (client == NULL) {
        return;
    }

    spdlog_debug("mqtt_client_destroy.");
    pthread_mutex_lock(&client->lock);
    MQTTAsync_disconnect(client->handle, NULL);
    MQTTAsync_destroy(&client->handle);
    pthread_mutex_unlock(&client->lock);
    pthread_mutex_destroy(&client->lock);
    free(client);
    client = NULL;
}

