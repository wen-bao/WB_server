#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "list.h"
#include "util.h"

#include <errno.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#define WB_HTTP_PARSE_INVALID_METHOD  10
#define WB_HTTP_PARSE_INVALID_REQUEST 11
#define WB_HTTP_PARSE_INVALID_HEADER  12

#define WB_HTTP_UNKNOWN               0x0001
#define WB_HTTP_GET                   0x0002
#define WB_HTTP_HEAD                  0x0004
#define WB_HTTP_POST                  0x0008

#define WB_HTTP_OK                    200
#define WB_HTTP_NOT_MODIFIED          304
#define WB_HTTP_NOT_FOUND             404
#define MAX_BUF                       8124

#define WB_AGAIN EAGAIN


typedef struct wb_http_request {
    char *root;
    int fd;
    int epoll_fd;
    char buff[MAX_BUF];
    size_t pos;
    size_t last;
    int state;

    void *request_start;
    void *method_end;
    int method;
    void *uri_start;
    void *uri_end;
    void *path_start;
    void *path_end;
    int http_major;
    int http_minor;
    void *request_end;

    struct list_head list;

    void *cur_header_key_start;
    void *cur_header_key_end;
    void *cur_header_value_start;
    void *cur_header_value_end;
    void *timer;
} wb_http_request_t;

typedef struct wb_http_out {
    int fd;
    int keep_alive;
    time_t mtime;
    int modified;
    int status;
} wb_http_out_t;

typedef struct wb_http_header {
    void *key_start;
    void *key_end;
    void *value_start;
    void *value_end;
    struct list_head list;
} wb_http_header_t;

typedef int (*wb_http_header_handler_pt)(wb_http_request_t *request, wb_http_out_t *out, char *data, int len);

typedef struct wb_http_header_handle {
    char *name;
    wb_http_header_handler_pt handler;
} wb_http_header_handle_t;

extern wb_http_header_handle_t wb_http_headers_in[];

void wb_http_handle_header(wb_http_request_t *request, wb_http_out_t *out);
int wb_http_close_conn(wb_http_request_t *request);
int wb_init_request_t(wb_http_request_t *request, int fd, int epoll_fd, char *path);
int wb_init_out_t(wb_http_out_t *out, int fd);
const char *get_shortmsg_from_status_code(int status_code);


#endif // !HTTP_REQUEST_H