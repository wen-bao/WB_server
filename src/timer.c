#include "timer.h"
#include <sys/time.h>

wb_pq_t wb_timer;
size_t wb_current_msec;

int timer_comp(void *ti, void *tj) {
    wb_timer_t *timeri = (wb_timer_t *) ti;
    wb_timer_t *timerj = (wb_timer_t *) tj;
    return (timeri->key < timerj->key) ? 1 : 0;
}

void wb_time_update() {
    struct timeval tv;
    int rc = gettimeofday(&tv, NULL);
    wb_current_msec = ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

int wb_timer_init() {
    int rc = wb_pq_init(&wb_timer, timer_comp, WB_PQ_DEFAULT_SIZE);

    wb_time_update();
    return 0;
}

int wb_find_timer() {
    int time;
    while(!wb_pq_is_empty(&wb_timer)) {
        wb_time_update();

        wb_timer_t *timer_node = (wb_timer_t *)wb_pq_min(&wb_timer);
        if(timer_node->deleted) {
            int rc = wb_pq_delmin(&wb_timer);
            free(timer_node);
            continue;
        }

        time = (int) (timer_node->key - wb_current_msec);
        time = (time > 0) ? time : 0;
        break;
    }
    return time;
}

void wb_handle_expire_timers() {
    while(!wb_pq_is_empty(&wb_timer)) {
        wb_time_update();
        wb_timer_t *timer_node = (wb_timer_t *)wb_pq_min(&wb_timer);

        if(timer_node->deleted) {
            int rc = wb_pq_delmin(&wb_timer);
            free(timer_node);
            continue;
        }

        if(timer_node->key > wb_current_msec) {
            return ;
        }

        if(timer_node->handler) {
            timer_node->handler(timer_node->request);
        }

        int rc = wb_pq_delmin(&wb_timer);
        free(timer_node);
    }
}

void wb_add_timer(wb_http_request_t *request, size_t timeout, timer_handler_pt handler) {
    wb_time_update();

    wb_timer_t *timer_node = (wb_timer_t *)malloc(sizeof(wb_timer_t));

    request->timer      = timer_node;
    timer_node->key     = wb_current_msec + timeout;
    timer_node->deleted = 0;
    timer_node->handler = handler;
    timer_node->request = request;

    int rc = wb_pq_insert(&wb_timer, timer_node);
}

void wb_del_timer(wb_http_request_t *request) {
    wb_time_update();
    wb_timer_t *timer_node = request->timer;
    timer_node->deleted = 1;
}