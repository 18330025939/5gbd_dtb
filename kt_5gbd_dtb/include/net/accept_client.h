#ifndef __ACCEPT_CLIENT_H
#define __ACCEPT_CLIENT_H

typedef struct st_AcceptClient AcceptClient;

#define MAX_TIMEOUT_TIME  2

struct AcceptClientOps {

    uint32_t (*get_addr)(AcceptClient* self);
    // int (*send_)
};

struct st_AcceptClient {

    struct event_base *base;
    struct bufferevent *bev;
    evutil_socket_t fd;
    struct sockaddr_in addr;
    struct AcceptClientOps *ops;
    int is_close;
} ;

AcceptClient* accept_client_create(struct event_base *base, evutil_socket_t fd, struct sockaddr_in *addr);

#endif

