#include "http_request.h"

static int wb_http_process_ignore(wb_http_request_t *request, wb_http_out_t *out, char *date, int len);
static int wb_http_process_connection(wb_http_request_t *request, wb_http_out_t *out, char *data, int len);
static int wb_http_process_if_modified_since(wb_http_request_t *request, wb_http_out_t *out, char *data, int len);

wb_http_header_handle_t wb_http_headers_in[] = {
    {"Host", wb_http_process_ignore},
    {"Connection", wb_http_process_connection},
    {"If-Modified-Since", wb_http_process_if_modified_since},
    {"", wb_http_process_ignore}
};

static int wb_http_process_ignore(wb_http_request_t *request, wb_http_out_t *out, char *data, int len) {
    (void) request;
    (void) out;
    (void) data;
    (void) len;
    return 0;
}

static int wb_http_process_connection(wb_http_request_t *request, wb_http_out_t *out, char *data, int len) {
    (void) request;
    if(strncasecmp("keep-alive", data, len) == 0) {
        out->keep_alive = 1;
    }
    return 0;
}

static int wb_http_process_if_modified_since(wb_http_request_t *request, wb_http_out_t *out, char *data, int len) {
    (void) request;
    (void) len;
    struct tm tm;

    if(strptime(data, "%a, %d %b %Y %H:%M%S GMT", &tm) == (char *) NULL) {
        return 0;
    }

    time_t client_time = mktime(&tm);
    double time_diff = difftime(out->mtime, client_time);
    if(fabs(time_diff) < 1e-6) {
        out->modified = 0;
        out->status = WB_HTTP_NOT_MODIFIED;
    }
    return 0;
}

int wb_init_request_t(wb_http_request_t *request, int fd, int epoll_fd, char *path) {
    request->fd       = fd;
    request->epoll_fd = epoll_fd;
    request->pos      = 0;
    request->last     = 0;
    request->state    = 0;
    request->root     = path;
    INIT_LIST_HEAD(&(request->list));
    return 0;
}

int wb_init_out_t(wb_http_out_t *out, int fd) {
    out->fd         = fd;
    out->keep_alive = 1;
    out->modified   = 1;
    out->status     = 200;
    return 0;
}

void wb_http_handle_header(wb_http_request_t *request, wb_http_out_t *out) {
    list_head *pos;
    wb_http_header_t *hd;
    wb_http_header_handle_t *header_in;
    int len;
    list_for_each(pos, &(request->list)) {
        hd = list_entry(pos, wb_http_header_t, list);
        for(header_in = wb_http_headers_in; strlen(header_in->name) > 0; header_in++) {
            if(strncmp(hd->key_start, header_in->name, hd->key_end - hd->key_start) == 0) {
                len = hd->value_end - hd->value_start;
                (*(header_in->handler))(request, out, hd->value_start, len);
                break;
            }
        }
        list_del(pos);
        free(hd);
    }
}

const char *get_shortmsg_from_status_code(int status_code) {
    if(status_code == WB_HTTP_OK) {
        return "OK";
    }
    if(status_code == WB_HTTP_NOT_MODIFIED) {
        return "Not Modified";
    }
    if(status_code == WB_HTTP_NOT_FOUND) {
        return "Not Found";
    }
    return "Unknown";
}

int wb_http_close_conn(wb_http_request_t *request) {
    close(request->fd);
    free(request);
    return 0;
}