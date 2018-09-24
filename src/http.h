#ifndef HTTP_H
#define HTTP_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

#include "epoll.h"
#include "timer.h"
#include "http_request.h"
#include "http_parse.h"
#include "rio.h"
#include "util.h"

#define MAXLINE     8192
#define SHORTLINE   512

#define wb_str3_cmp(m, c0, c1, c2, c3)                                  \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define wb_str30cmp(m, c0, c1, c2, c3)                                  \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define wb_str4cmp(m, c0, c1, c2, c3)                                   \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

typedef struct mime_type {
    const char *type;
    const char *value;
} mime_type_t;

void do_request(void *ptr);

#endif // !HTTP_H