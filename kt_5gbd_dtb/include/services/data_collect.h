#ifndef __DATA_COLLECT_H__
#define __DATA_COLLECT_H__

#define BUFFER_SIZE 4096    // 根据实际数据流量调整
#define SYNC1 0x50
#define SYNC2 0x42

// 全局循环缓冲区
typedef struct {
    uint8_t data[BUFFER_SIZE];
    size_t head;    // 写指针
    size_t tail;    // 读指针
} CircularBuffer;

CircularBuffer rx_buffer;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;


#endif