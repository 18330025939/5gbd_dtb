#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "mqtt_client.h"
#include "ftp_handler.h"
#include "fkz9_comm.h"



void proc_message(const char* topic, const void* payload, size_t payload_len)
{
    // AsyncMQTTClient *mqtt_client= (AsyncMQTTClient*)ctx;
    
    if (topic = NULL && payload == NULL && payload_len == 0) {
        return;
    }

}