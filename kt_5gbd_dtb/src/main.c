#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <event2/event.h>
#include "cloud_comm.h"

void signal_handler(evutil_socket_t fd, short events, void *arg)
{
    struct event_base *base = NULL;

    base = (struct event_base *)arg;
    printf("Received SIGINT,quit...\n");
    event_base_loopexit(base, NULL);
}

int main(int argc, char ** args)
{
    CloundCommContext *cloud_ctx = NULL;


    cloud_ctx = (CloundCommContext*)malloc(sizeof(CloundCommContext));
    if (cloud_ctx == NULL) {
        exit(1);
    }
    memset(cloud_ctx, 0, sizeof(CloundCommContext));
    
    // signal(SIGINT, signal_handler);
    clound_comm_init(cloud_ctx);

    struct event_base *base = event_base_new();
    if (!base) {
        perror("event_base_new");
        exit(1);
    }
    struct event *signal = evsignal_new(base, SIGINT, signal_handler, base);
    if (!signal) {
        perror("event_new");
        event_base_free(base);
        exit(1);
    }
    event_add(signal, NULL);

    event_base_dispatch(base);
    printf("wait....\n");
    event_free(signal);
    event_base_free(base);
    clound_comm_uninit(cloud_ctx);

    return 0;
}
