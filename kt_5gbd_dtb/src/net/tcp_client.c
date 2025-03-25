#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include "tcp_client.h"


// 重连机制
void tcp_client_reconnect(evutil_socket_t fd, short event, void *arg)
{
    TcpClient* client = (TcpClient*)arg;
    if (client->recnt_att < client->max_recnt_att) {
        client->recnt_att++;
        printf("Reconnecting... Attempt %d/%d\n", client->recnt_att, client->max_recnt_att);
        struct timeval timeout = {1, 0}; // 1秒后重试
        event_base_once(client->base, -1, EV_TIMEOUT, tcp_client_reconnect, client, &timeout);
    } else {
        printf("Max reconnect attempts reached. Exiting.\n");
        tcp_client_destroy(client);
        exit(1);
    }
}

// 服务器消息回调函数
static void tcp_client_read_cb(struct bufferevent* bev, void* arg)
{
    TcpClient* client = (TcpClient*)arg;
    char msg[1024];
    size_t len = bufferevent_read(bev, msg, sizeof(msg));
    msg[len] = '\0';
    printf("Received: %s\n", msg);
    if (client->on_message) {
        client->on_message(msg, len);
    }
}

// 事件回调函数
static void tcp_client_event_cb(struct bufferevent* bev, short event, void* arg)
{
    TcpClient* client = (TcpClient*)arg;
    if (event & BEV_EVENT_CONNECTED) {
        printf("Connected to server.\n");
        client->recnt_att = 0;
    } else if (event & BEV_EVENT_EOF) {
        printf("Connection closed.\n");
        tcp_client_reconnect(-1, EV_TIMEOUT, client);
    } else if (event & BEV_EVENT_ERROR) {
        printf("Error on connection.\n");
        tcp_client_reconnect(-1, EV_TIMEOUT, client);
    }
}

// 初始化客户端
int tcp_client_init(TcpClient* client)
{
    client->base = event_base_new();
    if (!client->base) {
        perror("event_base_new failed");
        return -1;
    }

    client->bev = bufferevent_socket_new(client->base, -1, BEV_OPT_CLOSE_ON_FREE);
    if (!client->bev) {
        perror("bufferevent_socket_new failed");
        goto base_failed;
    }

    // client->ev_cmd = event_new(client->base, STDIN_FILENO, EV_READ | EV_PERSIST, NULL, client);
    // if (!client->ev_cmd) {
    //     perror("event_new failed");
    //     goto event_failed;
    // }

    bufferevent_setcb(client->bev, tcp_client_read_cb, NULL, tcp_client_event_cb, client);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client->port);
    inet_aton(client->server_ip, &server_addr.sin_addr);

    if (bufferevent_socket_connect(client->bev, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bufferevent_socket_connect failed");
        goto bev_failed;
    }
    
    bufferevent_enable(client->bev, EV_READ | EV_WRITE | EV_PERSIST);

    event_base_dispatch(client->base);
    return 0;
    
bev_failed:
//     event_free(client->ev_cmd);
// event_failed:
    bufferevent_free(client->bev);
base_failed:
    event_base_free(client->base);
    return -1;
}

// 发送数据
void tcp_client_send(TcpClient* client, const char* data, size_t len) 
{
    bufferevent_write(client->bev, data, len);
}

TcpClient* tcp_client_create(const char* server_ip, int port, int max_recnt)
{
    TcpClient* client = (TcpClient*)malloc(sizeof(TcpClient));
    memset(client, 0, sizeof(TcpClient));
    client->server_ip = strdup(server_ip);
    client->port = port;
    client->max_recnt_att = max_recnt;
    client->recnt_att = 0;
    return client;
}

// 销毁客户端
void tcp_client_destroy(TcpClient* client) 
{
    // if (client->ev_cmd) {
    //     event_free(client->ev_cmd);
    // }
    if (client->bev) {
        bufferevent_free(client->bev);
    }
    if (client->base) {
        event_base_free(client->base);
    }
    free(client->server_ip);
    free(client);
}
