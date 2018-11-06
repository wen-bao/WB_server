#include "priority_queue.h"
#include <stdlib.h>
#include <string.h>

void exch(wb_pq_t *wb_pq, size_t i, size_t j) {
    void *tmp = wb_pq->pq[i];
    wb_pq->pq[i] = wb_pq->pq[j];
    wb_pq->pq[j] = tmp;
}

void swim(wb_pq_t *wb_pq, size_t k) {
    while(k > 1 && wb_pq->comp(wb_pq->pq[k], wb_pq->pq[k / 2])) {
        exch(wb_pq, k, k / 2);
        k /= 2;
    }
}

int sink(wb_pq_t *wb_pq, size_t k) {
    size_t j;
    size_t nalloc = wb_pq->nalloc;
    while((k << 1) <= nalloc) {
        j = k << 1;
        if((j < nalloc) && (wb_pq->comp(wb_pq->pq[j + 1], wb_pq->pq[j]))) {
            j ++;
        }

        if(!wb_pq->comp(wb_pq->pq[j], wb_pq->pq[k])) {
            break;
        }

        exch(wb_pq, j, k);
        k = j;
    }
    return k;
}

int wb_pq_sink(wb_pq_t *wb_pq, size_t i) {
    return sink(wb_pq, i);
}

int wb_pq_init(wb_pq_t *wb_pq, wb_pq_comparator_pt comp, size_t size) {
    wb_pq->pq = (void **) malloc(sizeof(void *) * (size + 1));
    if(!wb_pq->pq) {
        return -1;
    }

    wb_pq->nalloc = 0;
    wb_pq->size = size + 1;
    wb_pq->comp = comp;
    return 0;
}

int wb_pq_is_empty(wb_pq_t *wb_pq) {
    return (wb_pq->nalloc == 0) ? 1 : 0;
}

void *wb_pq_min(wb_pq_t *wb_pq) {
    if(wb_pq_is_empty(wb_pq)) {
        return (void *)(-1);
    }
    return wb_pq->pq[1];
}

int resize(wb_pq_t *wb_pq, size_t new_size) {
    if(new_size < wb_pq->nalloc) {
        return -1;
    }

    void **new_ptr = (void **)malloc(sizeof(void *) *new_size);
    if(!new_ptr) {
        return -1;
    }
    memcpy(new_ptr, wb_pq->pq, sizeof(void *) * (wb_pq->nalloc + 1));

    free(wb_pq->pq);
    wb_pq->pq = new_ptr;
    wb_pq->size = new_size;
    return 0;
}

int wb_pq_delmin(wb_pq_t *wb_pq) {
    if(wb_pq_is_empty(wb_pq)) {
        return 0;
    }
    exch(wb_pq, 1, wb_pq->nalloc);
    --wb_pq->nalloc;
    sink(wb_pq, 1);
    if((wb_pq->nalloc > 0) && (wb_pq->nalloc <= (wb_pq->size - 1) / 4)) {
        if(resize(wb_pq, wb_pq->size / 2) < 0) {
            return -1;
        }
    }
    return 0;
}

int wb_pq_insert(wb_pq_t *wb_pq, void *item) {
    if(wb_pq->nalloc + 1 == wb_pq->size) {
        if(resize(wb_pq, wb_pq->size * 2) < 0) {
            return -1;
        }
        wb_pq->pq[++wb_pq->nalloc] = item;
        swim(wb_pq, wb_pq->nalloc);
        return 0;
    }
}