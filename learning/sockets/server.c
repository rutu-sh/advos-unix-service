#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>

#include "connection.h"

typedef struct epoll_event epoll_event;

void set_nonblocking(int);  //set fd to non-blocking

int main() {
    int                conn_sock;
    int                data_sock;
    int                ret;
    int                result;
    int                epoll_fd;
    char               buffer[BUFFER_SIZE];
    ssize_t            r,w;
    struct sockaddr_un name;
    epoll_event ev, all_events[MAX_CONNECTIONS];

    conn_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( conn_sock == -1 ) {
        perror("socket creation\n");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_NAME);

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = bind( conn_sock, (const struct sockaddr*) &name, sizeof(name) );
    if ( ret == -1 ) {
        perror("bind\n");
        exit(EXIT_FAILURE);
    }

    ret = listen(conn_sock, 2);
    if( ret == -1 ) {
        perror("listen\n");
        exit(EXIT_FAILURE);
    }

    set_nonblocking(conn_sock);

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(conn_sock);
        exit(EXIT_FAILURE);
    }


    ev.events = EPOLLIN;
    ev.data.fd = conn_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
        perror("epoll_ctl: conn_sock");
        close(conn_sock);
        exit(EXIT_FAILURE);
    }

    for(;;) {
        int client_fd;
        int no_fds = epoll_wait(epoll_fd, all_events, NO_EVENTS, -1);
        if (no_fds == -1) {
            perror("epoll_wait err");
            break;
        }

        for (int i = 0; i < no_fds; i++) {

            int fd = all_events[i].data.fd;

            // if server gets accept req
            if (fd == conn_sock) {
                while ((client_fd = accept(conn_sock, NULL, NULL)) != -1) {
                    set_nonblocking(client_fd);

                    // add new client to epoll
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                        perror("epoll_ctl err");
                        close(client_fd);
                    }

                    printf("accepted conn, fd is %d\n", client_fd);
                }

                continue;
            }

            // other events from client
            if (all_events[i].events & EPOLLIN) {
                // memset(buffer, 0, sizeof(buffer));
                r = read(fd, buffer, sizeof(buffer));
                if (r == 0) {
                    // no bytes read bruh, client hung up?
                    printf("client disconn\n");
                    perror("fd: %d client disconn\n");
                    close(fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                } else if (r == -1) {
                    // client err
                    perror("fd: %d client err\n");
                    close(fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                } else {
                    buffer[sizeof(buffer) - 1] = 0;

                    w = write(fd, buffer, sizeof(buffer));
                    if( w == -1 ) {
                        perror("write\n");
                        close(fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    }

                    printf("\nclient fd %d> %s", fd, buffer);
                    fflush(stdout);
                }
            }
        }
    }

    close(conn_sock);
    unlink(SOCKET_NAME);
    return 0;
}



void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
        exit(EXIT_FAILURE);
    }
}

