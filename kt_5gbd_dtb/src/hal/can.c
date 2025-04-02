#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h> 
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
// #include "queue.h"
#include "can.h"
#include "publib.h"

/*接收*/
static void* rx_thread_func(void* arg)
{
    CANDevice* dev = (CANDevice*)arg;
    struct can_frame frame;
    
    while (dev->running) {
        if (read(dev->sockfd, &frame, sizeof(frame)) <= 0) {
            continue;
        }

        if (dev->data_cb) {
            dev->data_cb(&frame);
        }
    }
    return NULL;
}

/*发送*/
static void* tx_thread_func(void* arg)
{
    CANDevice* dev = (CANDevice*)arg;
    uint8_t* data = NULL;
    size_t len, data_len;

    while (&dev->running) {
        while (!dequeue(&dev->tx_queue, data, &len)) {
           
            data_len = len - sizeof(RetransHeader);
            size_t count = data_len  / 8;
            if (data_len % 8)   {
                count += 1; 
            }

            for (int i = 0; i < count; i++) {
                struct can_frame frame = {
                    .can_id = dev->can_id,
                    .can_dlc = data_len > 8 ? 8 : data_len
                };

                memcpy(frame.data, data + sizeof(RetransHeader) + i * 8, frame.can_dlc);
                if (sizeof(frame) != write(dev->sockfd, &frame, sizeof(frame))) {
                    break;
                }
                data_len -= frame.can_dlc;
            }
            // 加入重传队列
            RetransHeader *hdr = (RetransHeader*)data;
            if ((hdr->retry_counts - 1) > 0) {
                getCurrentTime(&hdr->time, NULL);
                hdr->retry_counts -= 1;
                enqueue(&dev->retrans_queue, data, len);
            } 
        }
        usleep(1000);
    }
    return NULL;
}

/* 重传管理 */
static void check_retransmissions(CANDevice* dev)
{
    uint8_t* data = NULL;
    size_t len;
    
    while (1) {
        if (dequeue(&dev->retrans_queue, data, &len)) {
            continue;
        }
        RetransHeader *hdr = (RetransHeader*)data;
        if (BeTimeOutMN(&hdr->time, hdr->timeouts) == 0) {
            enqueue(&dev->tx_queue, data, len);
        }
        else {
            enqueue(&dev->retrans_queue, data, len);
        }
    }
}


/* 打开CAN设备 */
static int can_device_open(CANDevice *dev, const char *ifname)
{
    if ((dev->sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket create failed");
        return -1;
    }

    strcpy(dev->ifr.ifr_name, ifname);
    if (ioctl(dev->sockfd, SIOCGIFINDEX, &dev->ifr) < 0) {
        perror("Ioctl SIOCGIFINDEX failed");
        close(dev->sockfd);
        return -1;
    }

    dev->addr.can_family = AF_CAN;
    dev->addr.can_ifindex = dev->ifr.ifr_ifindex;

    setsockopt(dev->sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

    // if (ioctl(dev->sockfd, SIOCGIFMTU, &dev->ifr) < 0) {
	// 	perror("Ioctl SIOCGIFMTU failed");
    //     close(dev->sockfd);
	// 	return 1;
	// }

    // if (ifr.ifr_mtu != CANFD_MTU) {
	// 	printf("CAN interface ist not CAN FD capable - sorry.\n");
    //     close(dev->sockfd);
	// 	return 1;
	// }

    if (bind(dev->sockfd, (struct sockaddr *)&dev->addr, sizeof(dev->addr)) < 0) {
        perror("Bind failed");
        close(dev->sockfd);
        return -1;
    }

    // pthread_mutex_init(&dev->retrans_queue.lock, NULL);

    init_queue(&dev->tx_queue, MAX_DATA_SIZE);
    init_queue(&dev->retrans_queue, MAX_DATA_SIZE);
    
    // 创建通信线程
    dev->running = true;
    pthread_create(&dev->rx_thread, NULL, rx_thread_func, dev);
    pthread_create(&dev->tx_thread, NULL, tx_thread_func, dev);

    return 0;
}


static void can_device_send(CANDevice* dev, const uint8_t* data, size_t len, uint8_t number, uint16_t timeouts) 
{
    // struct timespec now;

    if (number && timeouts) {
        RetransHeader hdr = {
            .is_retrans = 1,
            .retry_counts = number,
            .timeouts = timeouts
        } ;
        getCurrentTime(&hdr.time, NULL);

        uint8_t *buf = malloc(len + sizeof(RetransHeader));
        memcpy(buf, (uint8_t*)&hdr, sizeof(RetransHeader));
        memcpy(buf + sizeof(RetransHeader), data, len);
        enqueue(&dev->tx_queue, buf, len + sizeof(RetransHeader));
        free(buf);
    }
    else {
        enqueue(&dev->tx_queue, data, len);
    }
}

static void can_device_register_callback(CANDevice* dev, CanDataCallback cb) 
{
    dev->data_cb = cb;
}

/* 设置接收过滤器 */
static int can_device_set_filter(CANDevice *dev, struct can_filter *rfilter) 
{
    return setsockopt(dev->sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, 
                     rfilter, sizeof(struct can_filter));
}

static void can_device_close(CANDevice* dev) 
{
    dev->running = false;
    pthread_join(dev->rx_thread, NULL);
    pthread_join(dev->tx_thread, NULL);
    
    clean_queue(&dev->tx_queue);
    if (dev->sockfd >0) {
        close(dev->sockfd);
    }
}

static struct CANDeviceOps can_ops = {
    .open = can_device_open,
    .close = can_device_close,
    .send = can_device_send,
    .set_filter = can_device_set_filter,
    .register_callback = can_device_register_callback
} ;


CANDevice* can_device_create(void) 
{
    CANDevice* dev = calloc(1, sizeof(CANDevice));
    
    dev->ops = &can_ops;
    dev->sockfd = -1;

    return dev;
}
