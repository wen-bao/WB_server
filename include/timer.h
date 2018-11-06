#ifndef WB_TIME_H
#define WB_TIME_H

#include "http_request.h"
#include "priority_queue.h"

#define TIMEOUT_DEFAULT 500

typedef int (*timer_handler_pt)(wb_http_request_t *request);

typedef struct wb_timer {
    size_t key;
    int deleted;
    timer_handler_pt handler;
    wb_http_request_t *request;
} wb_timer_t;

extern wb_pq_t wb_timer;
extern size_t wb_current_msec;

int wb_timer_init();
int wb_find_timer();
void wb_handler_expire_timers();
void wb_add_timer(wb_http_request_t *request, size_t timeout, timer_handler_pt handler);
void wb_del_timer(wb_http_request_t *request);
int timer_comp(void *ti, void *tj);

#endif //!WB_TIME_H