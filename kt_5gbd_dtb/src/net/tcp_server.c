#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <event2/thread.h>
#include <event2/event.h>
// #include "accept_client.h"
#include "tcp_server.h"

void tcp_accept_cb(evutil_socket_t fd, short event, void *arg)
{
    TcpServerSocket *t_tcp_socket = (TcpServerSocket*)arg;
    struct sockaddr_in t_client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    evutil_socket_t t_client_fd = accept(fd, (struct sockaddr *)&t_client_addr, &len);
    if (t_client_fd < 0) {
        return ;
    }

    AcceptClient *t_client = accept_client_create(t_tcp_socket->base, t_client_fd, &t_client_addr);
    if (t_tcp_socket->client) {

        if(t_tcp_socket->client->ops->get_addr(t_client) != t_client_addr.sin_addr.s_addr) {

            return ;
        }
        else {
            printf("tcp没有断开的情况下,收到相同ip的连接,关闭旧连接,启动新连接.");
        }
    }
    else {
        t_tcp_socket->client = t_client;
    }

    return ;
}

void tcp_server_entry(TcpServerSocket *self)
{
    evthread_use_pthreads();
    evutil_socket_t t_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_listen_socket_reuseable(t_listen_fd);

    struct sockaddr_in sin = {
        .sin_family = AF_INET,
        .sin_port = htons(self->tcp_port),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(t_listen_fd, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        return ;
    }

    if (listen(t_listen_fd, self->listen_backlog) < 0) {
        return ;
    }

    evutil_make_socket_nonblocking(t_listen_fd);

    self->base = event_base_new();
    struct event *t_listen_event = event_new(self->base, t_listen_fd, EV_READ | EV_PERSIST, tcp_accept_cb, self);
    event_add(t_listen_event, NULL);

    event_base_dispatch(self->base);

    event_free(t_listen_event);
    evutil_closesocket(t_listen_fd);
    event_base_free(self->base);

    return;
}