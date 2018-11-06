#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdlib.h>

#define WB_PQ_DEFAULT_SIZE 10

typedef int (*wb_pq_comparator_pt)(void *pi, void *pj);

typedef struct priority_queue {
    void **pq;
    size_t nalloc;
    size_t size;
    wb_pq_comparator_pt comp;
} wb_pq_t;

int wb_pq_init(wb_pq_t *wb_pq, wb_pq_comparator_pt comp, size_t size);
int wb_pq_is_empty(wb_pq_t *wb_pq);
size_t wb_pq_size(wb_pq_t *wb_pq);
void *wb_pq_min(wb_pq_t *wb_pq);
int wb_pq_delmin(wb_pq_t *wb_pq);
int wb_pq_insert(wb_pq_t *wb_pq, void *item);
int wb_pq_sink(wb_pq_t *wb_pq, size_t i);

#endif //!PRIORITY_QUEUE_H