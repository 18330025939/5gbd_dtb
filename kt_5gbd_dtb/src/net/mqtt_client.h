#ifndef __MQTT_CLIENT_H
#define __MQTT_CLIENT_H

#include <MQTTAsync.h>

#define QOS 0
#define KEEP_ALIVE_TIME 20

typedef struct st_AsyncMQTTClient AsyncMQTTClient;

// 异步客户端配置结构体
typedef struct {
    char *address;                 // MQTT代理服务器地址
    char *client_id;               // 客户端ID
    char *user_name;
    char *password;
    int qos;   
    int keep_alive;                // 保持连接时间，单位秒
    int clean_session;             // 是否清除会话
    int conn_timeout;              // 连接超时时间，单位毫秒
    int cmd_timeout;               // 命令超时时间，单位毫秒
} AsyncClientConfig;

/*---------------------- 回调函数类型定义 ----------------------*/
typedef void (*mqtt_event_callback)(void* ctx, int code, const char* msg);

typedef struct {
    int (*connect)(AsyncMQTTClient* client);
    int (*publish)(AsyncMQTTClient* client, const char* topic, const void* payload, size_t len);
    int (*subscribe)(AsyncMQTTClient* client, const char* topic);
    void (*register_cb)(AsyncMQTTClient* client, void (*cb)(const char*, const void*, size_t));
} AsyncClientOps;

void (*MqttMessageCallback)(const char*, const void*, size_t);
// 异步客户端结构体
struct st_AsyncMQTTClient {
    MQTTAsync handle;               // Paho异步客户端句柄
    AsyncClientConfig *config;      // 客户端配置
    AsyncClientOps *ops;

    void (*on_message)(const char* topic, const void* payload, size_t payload_len);
    
    int is_conn;                 
    pthread_mutex_t lock;                
} ;

AsyncMQTTClient* mqtt_client_create(AsyncClientConfig *config);//(const char *addr, const char *id, const char *username, const char *password) ;
void mqtt_client_destroy(AsyncMQTTClient* client);

#endif /* __MQTT_CLIENT_H */