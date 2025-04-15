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

#define SYS_RUN_LED_PIN_NUM 104  //GPIO3_B0
#define SYS_ERR_LED_PIN_NUM 103  //GPIO3_A7 3*32+(1-1)*8+7

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
    LedController sys_run, sys_err;
    led_init(&sys_run, SYS_RUN_LED_PIN_NUM);
    led_init(&sys_err, SYS_ERR_LED_PIN_NUM);
    led_set_low(&sys_run);
    led_set_low(&sys_err);
    // signal(SIGINT, signal_handler);
    clound_comm_init(cloud_ctx);
    fkz9_comm_init(fkz9_ctx);

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
    fkz9_comm_uninit(fkz9_ctx);

    return 0;
}
