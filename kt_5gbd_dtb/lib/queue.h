
#ifndef __QUEUE_H
#define __QUEUE_H

#define MAX_SEGMENTS 128

typedef struct st_ThreadSafeQueue {
    pthread_mutex_t mutex;
    uint8_t* buffers[MAX_SEGMENTS];
    size_t sizes[MAX_SEGMENTS];
    unsigned int head;
    unsigned int tail;
} ThreadSafeQueue;


int init_queue(ThreadSafeQueue* q, size_t size);
int enqueue(ThreadSafeQueue* q, uint8_t* data, size_t len);
int dequeue(ThreadSafeQueue* q, uint8_t* data, size_t* len);
int clean_queue(ThreadSafeQueue* q);

#endif /* __QUEUE_H */