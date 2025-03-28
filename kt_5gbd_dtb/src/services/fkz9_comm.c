#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "queue.h"
#include "mqtt_client.h"
#include "ftp_handler.h"
#include "fkz9_comm.h"



void proc_message(const char* topic, const void* payload, size_t payload_len)
{
    // AsyncMQTTClient *mqtt_client= (AsyncMQTTClient*)ctx;
    
    if (topic = NULL && payload == NULL && payload_len == 0) {
        return;
    }

    for () {

    }



}

void send_msg_entry(void *arg)
{
    CommContext *ctx = (CommContext*)arg;
    while (1) {

    }

}


void do_init(void)
{
    CommContext *ctx = malloc(sizeof(CommContext));
    
    if (malloc == NULL) {
        return ;
    }

    bzero(ctx, sizeof(CommContext));
    init_queue(ctx->tx_queue, MAX_MSG_SIZE);
    init_queue(ctx->re_queue, MAX_MSG_SIZE);


}