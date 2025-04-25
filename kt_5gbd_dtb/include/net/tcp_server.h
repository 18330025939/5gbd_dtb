#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

// #include "accept_client.h"
// extern typedef struct st_AcceptClient AcceptClient;
typedef struct st_AcceptClient AcceptClient;

typedef void (*TcpClientCanllback)(int sockfd, const char *data, size_t len);


uint32_t get_client_addr(void);
void disconnect_client(void);

typedef struct {
    uint16_t tcp_port;
    int listen_backlog;
    AcceptClient *client;
    struct event_base *base;
    TcpClientCanllback callback;
} TcpServerSocket;


#endif
