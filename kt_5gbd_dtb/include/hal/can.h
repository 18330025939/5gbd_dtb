#ifndef __CAN_H
#define __CAN_H

#include <stdint.h>
#include <stdbool.h>
#include <net/if.h>
#include <linux/can.h>
#include "queue.h"

#define MAX_SEGMENTS     32
#define RETRY_TIMEOUT_MS 500
#define MAX_RETRIES      3
#define MAX_DATA_SIZE    256


typedef struct st_CANDevice CANDevice;

struct CANDeviceOps {
    int (*open)(CANDevice *dev, const char *ifname);
    void (*send)(CANDevice *dev, const uint8_t *data, size_t len, uint8_t number, uint16_t timeouts);
    int (*set_filter)(CANDevice *dev, struct can_filter *rfilter);
    void (*close)(CANDevice *dev);
    void (*register_callback)(CANDevice *dev, void (*cb)(struct can_frame *));
} ;

typedef struct {
    pthread_mutex_t lock;
    uint8_t* buffers[MAX_SEGMENTS];
    size_t sizes[MAX_SEGMENTS];
    uint8_t head;
    uint8_t tail;
} LockFreeBuffer;

#pragma pack(push, 1)
// 重传管理
typedef struct {    
    uint8_t is_retrans;
    uint8_t retry_counts;
    uint16_t timeouts;
    struct timeval time;
} RetransHeader;
#pragma pack(pop)

// typedef void (*CanDataCallback)(const uint8_t* data, size_t len);
typedef void (*CanDataCallback)(struct can_frame *);

struct st_CANDevice {
    int sockfd;
    char ifname[16];
    struct sockaddr_can addr;
    struct ifreq ifr;
    uint32_t can_id;
    
    bool running;
    // 接收系统
    pthread_t rx_thread;
    CanDataCallback data_cb;
    
    // 发送系统
    pthread_t tx_thread;
    ThreadSafeQueue tx_queue;

    uint32_t seq_counter;
    struct CANDeviceOps *ops;
    ThreadSafeQueue retrans_queue; 
};

CANDevice* can_device_create(void) ;
#endif 

