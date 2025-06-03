#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H

#include <stdbool.h>
#include "queue.h"

#define MAX_RECONNECT_ATTEMPTS 3000

typedef struct st_TcpClient TcpClient;

typedef struct st_TcpClientOps
{
    void (*connect)(TcpClient* client);
    void (*send)(TcpClient* client, uint8_t* data, size_t len);
    void (*disconnect)(TcpClient* client);
    void (*register_cb)(TcpClient* client, void (*cb)(char *buf, size_t len));
} TcpClientOps;

struct st_TcpClient
{
    struct event_base* base;
    struct bufferevent* bev;
    struct event* ev_cmd;
    char* server_ip;
    int port;
    int recnt_att;
    int max_recnt_att;
    bool is_connected;
    bool is_init;
    uint8_t conn_num;
    pthread_t conn_thread;
    pthread_t send_thread;
    ThreadSafeQueue tx_queue;
    // ThreadSafeQueue rx_queue;
    TcpClientOps *ops;
    void (*on_message)(char *buf, size_t len);
} ;


// int tcp_client_init(TcpClient* client);
// void tcp_client_send(TcpClient* client, uint8_t* data, size_t len);
TcpClient* tcp_client_create(const char* server_ip, int port, int max_recnt);
void tcp_client_destroy(void);

#endif /* __TCP_CLIENT_H */
