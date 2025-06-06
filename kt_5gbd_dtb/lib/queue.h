
#ifndef __QUEUE_H
#define __QUEUE_H

#define MAX_SEGMENTS 64
#define MAX_DATA_LEN 1400

typedef struct st_ThreadSafeQueue {
    pthread_mutex_t mutex;
    // uint8_t buffers[MAX_SEGMENTS][MAX_DATA_LEN];
    uint8_t* buffers[MAX_SEGMENTS];
    uint16_t sizes[MAX_SEGMENTS];
    unsigned int head;
    unsigned int tail;
} ThreadSafeQueue;


int init_queue(ThreadSafeQueue* q, size_t size);
// int init_queue(ThreadSafeQueue* q);
int enqueue(ThreadSafeQueue* q, uint8_t* data, size_t len);
int dequeue(ThreadSafeQueue* q, uint8_t* data, size_t* len);
int clean_queue(ThreadSafeQueue* q);

#endif /* __QUEUE_H */