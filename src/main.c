#include <stdio.h>
#include "http.h"
#include "threadpool.h"

#define DEFAULT_CONFIG "wb.conf"

extern struct epoll_event *events;

char *conf_file = DEFAULT_CONFIG;

wb_conf_t conf;

int main(int argc, char **argv) {

    read_conf(conf_file, &conf);
    //printf("root:%s\nport:%d\nthread_num:%d\n", conf.root, conf.port, conf.thread_num);

    handle_for_sigpipe();

    int listen_fd = socket_bind_listen(conf.port);

    int rc = make_socket_non_blocking(listen_fd);

    int epoll_fd = wb_epoll_create(0);

    wb_http_request_t *request = (wb_http_request_t *)malloc(sizeof(wb_http_request_t));
    wb_init_request_t(request, listen_fd, epoll_fd, conf.root);
    wb_epoll_add(epoll_fd, listen_fd, request, (EPOLLIN | EPOLLET));

    wb_threadpool_t *tp = threadpool_init(conf.thread_num);

    wb_timer_init();

    //printf("WB > listen_fd = %d***rc = %d***epoll_fd = %d***\n", listen_fd, rc, epoll_fd);

    while(1) {
        int time = wb_find_timer();

        int events_num = wb_epoll_wait(epoll_fd, events, MAXEVENTS, -1);

        wb_handle_expire_timers();

        wb_handle_events(epoll_fd, listen_fd, events, events_num, conf.root, tp);
    }

    //threadpool_destroy(tp, graceful_shutdown);
    return 0;
}