#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <event2/event.h>
#include "cloud_comm.h"
#include "fkz9_comm.h"
#include "led.h"
#include "ssh_client.h"




void signal_handler(evutil_socket_t fd, short events, void *arg)
{
    struct event_base *base = NULL;
    int signal = fd;

    base = (struct event_base *)arg;
    printf("Received signal %s (signal %d),quit...\n", strsignal(signal), signal);
    event_base_loopexit(base, NULL);
}

int main(int argc, char ** args)
{
    CloundCommContext *cloud_ctx = NULL;
    Fkz9CommContext *fkz9_ctx = NULL;

    cloud_ctx = (CloundCommContext*)malloc(sizeof(CloundCommContext));
    if (cloud_ctx == NULL) {
        exit(1);
    }
    memset(cloud_ctx, 0, sizeof(CloundCommContext));
    
    fkz9_ctx = (Fkz9CommContext*)malloc(sizeof(Fkz9CommContext));
    if (fkz9_ctx == NULL) {
        free(cloud_ctx);
        exit(1);
    }
    RUN_LED_INIT();
    FAULT_LED_INIT();

    clound_comm_init(cloud_ctx);
    fkz9_comm_init(fkz9_ctx);

    struct event_base *base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not create event base: exiting\n");
        exit(1);
    }
    struct event *sigint = evsignal_new(base, SIGINT, signal_handler, base);
    if (!sigint) {
        fprintf(stderr, "Could not create SIGINT event: exiting\n");
        event_base_free(base);
        exit(1);
    }
    event_add(sigint, NULL);

    event *sigterm = evsignal_new(base, SIGTERM, signal_handler, base);
    if (!sigterm) {
        fprintf(stderr, "Could not create SIGTERM event: exiting\n");
        event_free(sigint);
        event_base_free(base);
        exit(1);
    }
    event_add(sigterm, NULL);

    event_base_dispatch(base);

    printf("wait....\n");
    event_free(signal);
    event_base_free(base);
    clound_comm_uninit(cloud_ctx); 
    fkz9_comm_uninit(fkz9_ctx);
    FAULT_LED_ON();

    exit(0);
}
