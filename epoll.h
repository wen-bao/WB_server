#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include "threadpool.h"
#include "http.h"

#define MAXEVENTS 1024

int wb_epoll_create(int flags);

#endif // !EPOLL_H