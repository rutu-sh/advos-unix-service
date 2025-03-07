#include <stdio.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <fcntl.h>

#include "client.h"
#include "common/errorcodes.h"
#include "common/passfd.h"
#define MAX_EVENTS 100

LogContext log_ctx;
int        data_sock;


int main() {
    int         ret;
    char        w_buffer[BUFFER_SIZE];
    char        r_buffer[BUFFER_SIZE];
    char        fr_buffer[BUFFER_SIZE];
    int         epoll_fd;
    int         nfds;
    sockaddr_un name;
    struct epoll_event ev, ev2, events[MAX_EVENTS];

    init();

    memset(fr_buffer, 0, sizeof(fr_buffer));
    data_sock = create_data_socket();
    if ( data_sock == ERROR_CLIENT_DATA_SOCKET_CREATE ) {
        log_error(&log_ctx, "error creating data socket\nExiting...\n");
        graceful_exit("err creating data sock\n", ERROR_CLIENT_DATA_SOCKET_CREATE);
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = connect(data_sock, (const sockaddr*)&name, sizeof(name));
    if (ret == -1) {
        perror("connect\n");
        exit(EXIT_FAILURE);
    }
    // set_nonblocking(data_sock);
    // set_nonblocking(STDIN_FILENO);

    // add data socket to epoll
    epoll_fd = create_epoll_fd();
    ev.events = EPOLLIN;
    ev.data.fd = data_sock;
    if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data_sock, &ev) == -1 ) {
        log_error(&log_ctx, "error epoll_ctl\n");
        graceful_exit("error epoll_ctl\n", ERROR_CLIENT_EPOLL_CTL);
    }

    // add stdin to epoll
    ev2.events = EPOLLIN;
    ev2.data.fd = STDIN_FILENO;
    if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev2) == -1 ) {
        log_error(&log_ctx, "error epoll_ctl\n");
        graceful_exit("error epoll_ctl\n", ERROR_CLIENT_EPOLL_CTL);
    }

    while(1) {
        printf("\x1b[36mclient> \x1b[0m");
        fflush(stdout);
        nfds  = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if ( nfds == -1 ) {
            log_error(&log_ctx, "error epoll_wait\n");
            graceful_exit("error epoll_wait\n", ERROR_CLIENT_EPOLL_CTL);
        }

        for (int i = 0; i < nfds; i++) {
            // poll for data on stdin
            if ( events[i].data.fd == STDIN_FILENO ) {
                //printf("\nclient> ");
                memset(w_buffer, 0, sizeof(w_buffer));
                if(fgets(w_buffer, sizeof(w_buffer), stdin) == NULL){
                    continue;
                }
                handle_stdin_event(w_buffer, fr_buffer);
            }
            // poll for data on the data socket
            else if ( events[i].data.fd == data_sock ) {
                  handle_datasock_event(r_buffer);
            }
        }    
    }
}