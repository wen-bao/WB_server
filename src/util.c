#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "util.h"
#include "http_request.h"
#include "epoll.h"

int read_conf(char *filename, wb_conf_t *conf) {
    FILE *fp = fopen(filename, "r");
    if(!fp) {
        return WB_CONF_ERROR;
    }

    char buff[BUFLEN];
    int buff_len = BUFLEN;
    char *curr_pos = buff;
    char *delim_pos = NULL;

    int i = 0;
    int pos = 0;
    int line_len = 0;
    while(fgets(curr_pos, buff_len - pos, fp)) {
        delim_pos = strstr(curr_pos, DELIM);
        if(!delim_pos) {
            return WB_CONF_ERROR;
        }

        if(curr_pos[strlen(curr_pos) - 1] == '\n') {
            curr_pos[strlen(curr_pos) - 1] = '\0';
        }

        if(strncmp("root", curr_pos, 4) == 0) {
            delim_pos = delim_pos + 1;
            while(*delim_pos != '#') {
                conf->root[i++] = *delim_pos;
                ++delim_pos;
            }
        }

        if(strncmp("port", curr_pos, 4) == 0) {
            conf->port = atoi(delim_pos + 1);
        }

        if(strncmp("thread_num", curr_pos, 9) == 0) {
            conf->thread_num = atoi(delim_pos + 1);
        }

        line_len = strlen(curr_pos);

        curr_pos += line_len;
    }

    fclose(fp);
    return WB_CONF_OK;
}

void handle_for_sigpipe() {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags   = 0;

    if(sigaction(SIGPIPE, &sa, NULL)) {
        return;
    }
}

int socket_bind_listen(int port) {
    port = ((port <= 1024) || (port >= 65535)) ? 6666 : port;

    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) == -1) {
        return -1;
    }
    //printf("****\n");

    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        return -1;
    }

    if(listen(listen_fd, LISTENQ) == -1) {
        return -1;
    }

    if(listen_fd == -1) {
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}


int make_socket_non_blocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1) {
        return -1;
    }

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1) {
        return -1;
    }
}

void accept_connection(int listen_fd, int epoll_fd, char *path) {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = 0;
    int accept_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if(accept_fd == -1) {
        perror("accept");
    }

    int rc = make_socket_non_blocking(accept_fd);

    wb_http_request_t *request = (wb_http_request_t *)malloc(sizeof(wb_http_request_t));
    wb_init_request_t(request, accept_fd, epoll_fd, path);

    wb_epoll_add(epoll_fd, accept_fd, request, (EPOLLIN | EPOLLET | EPOLLONESHOT));

    wb_add_timer(request, TIMEOUT_DEFAULT, wb_http_close_conn);
}