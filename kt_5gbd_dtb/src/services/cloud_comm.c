#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "queue.h"
#include "tcp_client.h"
#include "ftp_handler.h"
#include "cloud_comm.h"


uint16_t checkSum_8(uint8_t *buf, uint16_t len) //buf为数组，len为数组长度
{
    uint8_t i;
    uint16_t ret = 0;
    for(i=0; i<len; i++)
    {
        ret += *(buf++);
    }
//    ret = ~ret;
    return ret;
}