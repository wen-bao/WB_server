#include "http.h"
#include <errno.h>

static const char *get_file_type(const char *type);
static void parse_uri(char *uri, int length, char *filename, char *query);
static void do_error(int fd, char *cause, char *err_num, char *short_msg, char *long_msg);
static void serve_static(int fd, char *filename, size_t filesize, wb_http_out_t *out);

static char *ROOT = NULL;

static void parse_uri(char *uri_start, int uri_length, char *filename, char *query) {
    uri_start[uri_length] = '\0';

    char *delim_pos = strchr(uri_start, '?');
    int filename_length = (delim_pos != NULL) ? ((int)(delim_pos - uri_start)) : uri_length;
    strcpy(filename, ROOT);
    strncat(filename, uri_start, filename_length);
    char *last_comp = strrchr(filename, '/');
    char *last_dot = strrchr(last_comp, '.');
    if((last_dot == NULL) && (filename[strlen(filename) - 1] != '/')) {
        strcat(filename, "/");
    }

    if(filename[strlen(filename) - 1] == '/') {
        strcat(filename, "index.html");
    }
    printf("%s\n", filename);
    return ;
}

void do_error(int fd, char *cause, char *err_num, char *short_msg, char *long_msg) {
    char header[MAXLINE];
    char body[MAXLINE];

    sprintf(body, "<html><title>WB Error</title>");
    sprintf(body, "%s<body bgcolor = ""ffffff"">\n", body);
    sprintf(body, "%s%s : %s\n", body, err_num, short_msg);
    sprintf(body, "%s<p>%s : %s\n</p>", body, long_msg, cause);
    sprintf(body, "%s<hr><em>WB web server</em>\n</body></html>", body);

    sprintf(header, "HTTP/1.1 %s %s\r\n", err_num, short_msg);
    sprintf(header, "%sServer: WB\r\n", header);
    sprintf(header, "%sConten-type: text/html\r\n", header);
    sprintf(header, "%sConnection: close\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));

    rio_writen(fd, header, strlen(header));
    rio_writen(fd, body, strlen(body));
    return ;
}

int error_proess(struct stat *sbufptr, char *filename, int fd) {
    if(stat(filename, sbufptr) < 0) {
        do_error(fd, filename, "404", "Not Found", "WB can't find the file");
        return 1;
    }

    if(!(S_ISREG((*sbufptr).st_mode)) || !(S_IRUSR & (*sbufptr).st_mode)) {
        do_error(fd, filename, "403", "Forbidden", "WB can't read the file");
        return 1;
    }
    return 0;
}

mime_type_t wb_mime[] = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {NULL, "text/plain"}
};

const char *get_file_type(const char *type) {
    for(int i = 0; wb_mime[i].type != NULL; ++i) {
        if(strcmp(type, wb_mime[i].type) == 0) {
            return wb_mime[i].value;
        }
    }
    return "text/plain";
}

void server_static(int fd, char *filename, size_t filesize, wb_http_out_t *out) {
    char header[MAXLINE];
    char buff[SHORTLINE];
    struct tm tm;

    sprintf(header, "HTTP/1.1 %d %s\r\n", out->status, get_shortmsg_from_status_code(out->status));

    if(out->keep_alive) {
        sprintf(header, "%sConnection: keep-alive\r\n", header);
        sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, TIMEOUT_DEFAULT);
    }
    if(out->modified) {
        const char *filetype = get_file_type(strrchr(filename, '.'));
        sprintf(header, "%sContent-type: %s\r\n", header, filetype);

        sprintf(header, "%sContent-length: %zu\r\n", header, filesize);

        localtime_r(&(out->mtime), &tm);
        strftime(buff, SHORTLINE, "%a, %d %b %Y %H:%M:%S GMT", &tm);
        sprintf(header, "%sLast-Modified: %s\r\n", header, buff);
    }
    sprintf(header, "%sServer : WB\r\n", header);

    sprintf(header, "%s\r\n", header);

    size_t send_len = (size_t)rio_writen(fd, header, strlen(header));
    if(send_len != strlen(header)) {
        perror("Send header failed");
        return ;
    }

    int src_fd = open(filename, O_RDONLY, 0);
    char *src_addr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);

    send_len = rio_writen(fd, src_addr, filesize);
    if(send_len != filesize) {
        perror("Send file failed");
        return ;
    }
    munmap(src_addr, filesize);

}

void do_request(void *ptr) {
    wb_http_request_t *request = (wb_http_request_t *) ptr;
    int fd = request->fd;
    ROOT = request->root;
    char filename[SHORTLINE];
    struct stat sbuf;
    int rc, n_read;
    char *plast = NULL;
    size_t remain_size;

    wb_del_timer(request);

    while(1) {
        plast = &request->buff[request->last % MAX_BUF];

        remain_size = MIN(MAX_BUF - (request->last - request->pos) - 1, MAX_BUF - request->last % MAX_BUF);

        n_read = read(fd, plast, remain_size);

        if(n_read == 0) {
            goto err;
        }

        if((n_read < 0) && (errno != WB_AGAIN)) {
            goto err;
        }

        if((n_read < 0) && (errno == WB_AGAIN)) {
            break;
        }

        request->last += n_read;

        rc = wb_http_parse_request_line(request);
        if(rc == WB_AGAIN) {
            continue;
        } else if(rc != 0) {
            goto err;
        }

        rc = wb_http_parse_request_body(request);
        if(rc == WB_AGAIN) {
            continue;
        } else if(rc != 0) {
            goto err;
        }

        wb_http_out_t *out = (wb_http_out_t *)malloc(sizeof(wb_http_out_t));

        wb_init_out_t(out, fd);

        //printf("after parse line and body!!\n");
        parse_uri(request->uri_start, request->uri_end - request->uri_start, filename, NULL);

        if(error_proess(&sbuf, filename, fd)) {
            continue;
        }

        wb_http_handle_header(request, out);

        out->mtime = sbuf.st_mtime;

        server_static(fd, filename, sbuf.st_size, out);

        free(out);

        if(!out->keep_alive) {
            goto close;
        }
    }

    wb_epoll_mod(request->epoll_fd, request->fd, request, (EPOLLIN | EPOLLET | EPOLLONESHOT));
    wb_add_timer(request, TIMEOUT_DEFAULT, wb_http_close_conn);

err:
close:
    wb_http_close_conn(request);

}