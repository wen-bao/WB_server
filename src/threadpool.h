#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdlib.h>
#include <pthread.h>

typedef struct wb_task {
    void (*func)(void *);
    void *arg;
    struct wb_task *next;
} wb_task_t;

typedef struct threadpool {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t *threads;
    wb_task_t *head;
    int thread_count;
    int queue_size;
    int shutdown;
    int started;
} wb_threadpool_t;

typedef enum {
    wb_tp_invalid = -1,
    wb_tp_lock_fail = -2,
    wb_tp_already_shutdown = -3,
    wb_tp_cond_broadcast = -4,
    wb_tp_thread_fail = -5
} wb_threadpool_error_t;

typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown = 2
} wb_threadpool_sd_t;

wb_threadpool_t *threadpool_init(int thread_num);
int threadpool_add(wb_threadpool_t *pool, void (*func)(void *), void *arg);
int threadpool_destroy(wb_threadpool_t *pool, int gracegul);

#endif //!THREADPOOL_H