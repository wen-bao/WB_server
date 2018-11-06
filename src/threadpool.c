#include "threadpool.h"

static int threadpool_free(wb_threadpool_t *pool);
static void *threadpool_worker(void *arg);

int threadpool_free(wb_threadpool_t *pool) {
    if(pool == NULL || pool->started > 0) {
        return -1;
    }

    if(pool->threads) {
        free(pool->threads);
    }

    wb_task_t *old;
    while(pool->head->next) {
        old = pool->head->next;
        pool->head->next = pool->head->next->next;
        free(old);
    }
    return 0;
}

void *threadpool_worker(void *arg) {
    if(arg == NULL) {
        return NULL;
    }

    wb_threadpool_t *pool = (wb_threadpool_t *)arg;
    wb_task_t *task;

    while(1) {
        pthread_mutex_lock(&(pool->lock));

        while((pool->queue_size == 0) && !(pool->shutdown)) {
            pthread_cond_wait(&(pool->cond), &(pool->lock));
        }

        if(pool->shutdown == immediate_shutdown) {
            break;
        } else if((pool->shutdown == graceful_shutdown) && (pool->queue_size == 0)) {
            break;
        }

        task = pool->head->next;

        if(task == NULL) {
            pthread_mutex_unlock(&(pool->lock));
            continue;
        }

        pool->head->next = task->next;
        pool->queue_size --;
        pthread_mutex_unlock(&(pool->lock));

        (*(task->func))(task->arg);
        free(task);
    }
    pool->started --;
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return NULL;
}

int threadpool_destroy(wb_threadpool_t *pool, int graceful) {
    if(pool == NULL) {
        return wb_tp_invalid;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return wb_tp_lock_fail;
    }

    int err = 0;
    do {
        if(pool->shutdown) {
            err = wb_tp_already_shutdown;
            break;
        }

        pool->shutdown = (graceful) ? graceful_shutdown : immediate_shutdown;

        if(pthread_cond_broadcast(&(pool->cond)) != 0) {
            err = wb_tp_cond_broadcast;
            break;
        }

        if(pthread_mutex_unlock(&(pool->lock)) != 0) {
            err = wb_tp_lock_fail;
            break;
        }

        for(int i = 0; i < pool->thread_count; ++i) {
            if(pthread_join(pool->threads[i], NULL) != 0) {
                err = wb_tp_thread_fail;
            }
        }
    } while(0);


    if(!err) {
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->cond));
        threadpool_free(pool);
    }

    return err;
}

wb_threadpool_t *threadpool_init(int thread_num) {
    wb_threadpool_t *pool;
    if((pool = (wb_threadpool_t *)malloc(sizeof(wb_threadpool_t))) == NULL) {
        goto err;
    }

    pool->thread_count  = 0;
    pool->queue_size    = 0;
    pool->shutdown      = 0;
    pool->started       = 0;
    pool->threads       = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);

    pool->head = (wb_task_t *)malloc(sizeof(wb_task_t));
    if((pool->threads == NULL) || (pool->head == NULL)) {
        goto err;
    }

    pool->head->func    = NULL;
    pool->head->arg     = NULL;
    pool->head->next    = NULL;

    if(pthread_mutex_init(&(pool->lock), NULL) != 0) {
        goto err;
    }

    if(pthread_cond_init(&(pool->cond), NULL) != 0) {
        goto err;
    }

    for(int i = 0; i < thread_num; ++i) {
        if(pthread_create(&(pool->threads[i]), NULL, threadpool_worker, (void *) pool) != 0) {
            threadpool_destroy(pool, 0);
            return NULL;
        }
        pool->thread_count ++;
        pool->started ++;
    }
    return pool;

err:
    if(pool)
        threadpool_free(pool);
    return NULL;
}

int threadpool_add(wb_threadpool_t *pool, void (*func)(void *), void *arg) {
    int rc, err = 0;
    if(pool == NULL || func == NULL) {
        return -1;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return -1;
    }

    if(pool->shutdown) {
        err = wb_tp_already_shutdown;
        goto out;
    }

    wb_task_t *task = (wb_task_t *)malloc(sizeof(wb_task_t));
    if(task == NULL) {
        goto out;
    }
    task->func = func;
    task->arg = arg;

    task->next = pool->head->next;
    pool->head->next = task;
    pool->queue_size ++;

    rc = pthread_cond_signal(&(pool->cond));

out:
    if(pthread_mutex_unlock(&pool->lock) != 0) {
        return -1;
    }
    return err;
}