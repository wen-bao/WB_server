#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include "util.h"
#include "http_request.h"
#include "threadpool.h"
#include "http.h"
#include "priority_queue.h"

#define MAXEVENTS 1024

int wb_epoll_create(int flags);
int wb_epoll_add(int epoll_fd, int fd, wb_http_request_t *request, int events);
int wb_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout);
int wb_epoll_mod(int epoll_fd, int fd, wb_http_request_t *request, int events);
void wb_handle_events(int epoll_fd, int listen_fd, struct epoll_event *events, int events_num, char *path, wb_threadpool_t *tp);

#endif // !EPOLL_H