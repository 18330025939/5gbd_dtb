#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H

#define MAX_RECONNECT_ATTEMPTS 3

typedef struct {
    struct event_base* base;
    struct bufferevent* bev;
    struct event* ev_cmd;
    char* server_ip;
    int port;
    int recnt_att;
    int max_recnt_att;
    void (*on_message)(char *buf, size_t len);
} TcpClient;

TcpClient* tcp_client_create(const char* server_ip, int port, int max_recnt);
void tcp_client_destroy(TcpClient* client);
#endif
