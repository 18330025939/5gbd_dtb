#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "queue.h"

int init_queue(ThreadSafeQueue* q, size_t size)
{
    pthread_mutex_init(&q->mutex, NULL);
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        q->buffers[i] = (uint8_t*)malloc(size * sizeof(uint8_t));
        bzero(q->buffers[i], size * sizeof(uint8_t));
    }

    bzero(q->sizes, size * sizeof(uint8_t));
    q->head = 0;
    q->tail = 0;
    return 0;
}

// 入队操作
int enqueue(ThreadSafeQueue* q, const uint8_t* data, size_t len) 
{
    pthread_mutex_lock(&q->mutex);
    
    unsigned int next = (q->head + 1) % MAX_SEGMENTS;
    if (next == q->tail) {
        pthread_mutex_unlock(&q->mutex);
        return -1; // 队列满
    }
    
    memcpy(q->buffers[q->head], data, len);
    q->sizes[q->head] = len;
    q->head = next;
    
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

// 出队操作
int dequeue(ThreadSafeQueue* q, uint8_t* data, size_t* len) 
{
    pthread_mutex_lock(&q->mutex);
    
    if (q->tail == q->head) {
        pthread_mutex_unlock(&q->mutex);
        return -1; // 队列空
    }
    
    memcpy(data, q->buffers[q->tail], q->sizes[q->tail]);
    *len = q->sizes[q->tail];
    q->tail = (q->tail + 1) % MAX_SEGMENTS;
    
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

int clean_queue(ThreadSafeQueue* q)
{
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        free(q->buffers[i]);
    }
    return 0;
}

