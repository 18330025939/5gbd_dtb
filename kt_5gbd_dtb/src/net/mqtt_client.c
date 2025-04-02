#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "mqtt_client.h"


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
    
    printf("Successful connection\n");
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
    printf("Connect failed, rc %d\n", response ? response->code : 0);
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
    AsyncClientConfig *config = (AsyncClientConfig *)&client->config;
    
    // 配置连接参数
    MQTTAsync_connectOptions opts = MQTTAsync_connectOptions_initializer;
    opts.keepAliveInterval = config->keep_alive;
    opts.cleansession = config->clean_session;
    opts.username = config->user_name;
    opts.password = config->password;
    opts.context = client;
    opts.onSuccess = on_connect_success;
    opts.onFailure = on_connect_failure;
    
    pthread_mutex_lock(&client->lock);
    int rc = MQTTAsync_connect(client->handle, &opts);
    pthread_mutex_unlock(&client->lock);
    return rc;
}

/* 消息发布 */
int mqtt_publish(AsyncMQTTClient* client, const char* topic, const void* payload, size_t len) 
{
    AsyncClientConfig *config = (AsyncClientConfig *)&client->config;
    
    MQTTAsync_message msg = MQTTAsync_message_initializer;
    // MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
    msg.payload = (void*)payload;
    msg.payloadlen = len;
    msg.qos = config->qos;
    msg.retained = 0;

    pthread_mutex_lock(&client->lock);
    int rc = MQTTAsync_sendMessage(client->handle, topic, &msg, NULL);
    pthread_mutex_unlock(&client->lock);
    return rc;
}

static void on_subscribe_success(void* context, MQTTAsync_successData* response)
{
	printf("Subscribe succeeded\n");
}

static void on_subscribe_failure(void* context, MQTTAsync_failureData* response)
{
	printf("Subscribe failed, rc %d\n", response->code);
}

/* 消息订阅 */
int mqtt_subscribe(AsyncMQTTClient* client, const char* topic) 
{
    AsyncClientConfig *config = (AsyncClientConfig *)&client->config;
    
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = on_subscribe_success;
	opts.onFailure = on_subscribe_failure;
	opts.context = client;

    pthread_mutex_lock(&client->lock);
    int rc = MQTTAsync_subscribe(client->handle, topic, config->qos, &opts);
    pthread_mutex_unlock(&client->lock);
    return rc;
}

static void mqtt_register_cb(AsyncMQTTClient* client, MqttMessageCallback *cb)
{
    client->on_message = cb;
}

static AsyncClientOps client_ops = {
    .connect = mqtt_connect,
    .publish = mqtt_publish,
    .subscribe = mqtt_subscribe,
    .register_cb = mqtt_register_cb
} ;

static AsyncClientConfig client_config = {
    .keep_alive = KEEP_ALIVE_TIME,
    .qos = QOS,
    .clean_session = 1,
} ;

/* 初始化 */
AsyncMQTTClient* mqtt_client_create(const char *addr, const char *id, const char *username, const char *password) 
{
    int rc;
    AsyncMQTTClient* client = calloc(1, sizeof(AsyncMQTTClient));
    pthread_mutex_init(&client->lock, NULL);

    client->config = &client_config;

    client->config->address = strdup(addr);
    client->config->client_id = strdup(id);
    client->config->user_name = strdup(username);
    client->config->password = strdup(password);

    client->ops = &client_ops;
    client->is_conn = false;

    if ((rc = MQTTAsync_create(&client->handle, client->config->address, client->config->client_id, 
                        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS) {
        printf("Failed to create client object, return code %d\n", rc);
        goto exit;
    }

    if ((rc = MQTTAsync_setCallbacks(client->handle, client, NULL, message_arrived_cb,  
                        NULL)) != MQTTASYNC_SUCCESS) {
        printf("Failed to set callback, return code %d\n", rc);
        goto exit;                    
    }
    return client;

exit:
    mqtt_client_destroy(client);
    return NULL;
}

/* 销毁 */
void mqtt_client_destroy(AsyncMQTTClient* client) 
{
    if (!client) {
        return;
    }

    pthread_mutex_lock(&client->lock);
    MQTTAsync_disconnect(client->handle, NULL);
    MQTTAsync_destroy(&client->handle);
    pthread_mutex_unlock(&client->lock);
    pthread_mutex_destroy(&client->lock);
    free(client->config->address);
    free(client->config->client_id);
    free(client->config->user_name);
    free(client->config->password);
    free(client);
    client = NULL;
}