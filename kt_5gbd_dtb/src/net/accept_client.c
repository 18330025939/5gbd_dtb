#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <event2/thread.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include "accept_client.h"

void client_read_cb(struct bufferevent *bev, void *arg)
{
    AcceptClient* client = (AcceptClient*)arg;
    size_t recv_len;
    uint8_t buf[4096];
    while (true) {
        recv_len = bufferevent_read(client->bev, buf, sizeof(buf));
        if (recv_len <= 0) {
            break;
        }


    }
}

void client_write_cb(struct bufferevent *bev, void *arg)
{

    return ;
}

void client_event_cb(struct bufferevent *bev, short events, void *arg)
{
    AcceptClient* client = (AcceptClient*)arg;
    if (client->is_close) {
        return;
    }

    if (events & BEV_EVENT_TIMEOUT) {

    }
    else if (events & BEV_EVENT_EOF) {

    }
    else {

    }

    client->is_close = true;
}

uint32_t client_get_addr(AcceptClient* slef)
{
    return slef->addr.sin_addr.s_addr;
}

static struct AcceptClientOps client_ops = {
    .get_addr = client_get_addr
} ;

AcceptClient* accept_client_create(struct event_base *base, evutil_socket_t fd, struct sockaddr_in *addr)
{
    AcceptClient* client = malloc(sizeof(AcceptClient));
    memset(client, 0, sizeof(AcceptClient));

    client->ops = &client_ops;
    client->is_close = false;
    client->bev = bufferevent_socket_new(base, fd, BEV_OPT_THREADSAFE | BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(client->bev, client_read_cb, client_write_cb, client_event_cb, client);
    bufferevent_enable(client->bev, EV_READ | EV_WRITE | EV_PERSIST);

    struct timeval t_timeout = {MAX_TIMEOUT_TIME, 0};
    bufferevent_set_timeouts(client->bev, &t_timeout, &t_timeout);

    return client;
}