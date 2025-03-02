#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "server.h"
#include "common/passfd.h"
#include "common/errorcodes.h"


int connections[MAX_CONNECTIONS];
char buffer[BUFFER_SIZE];
LogContext log_ctx;
epoll_event ev;
epoll_event all_events[MAX_CONNECTIONS];
int conn_sock;

int main() {

    int         ret;
    int         epoll_fd;
    int         client_fd;
    int         no_fds;
    int         idx;
    int         event_fd;
    int         read_bytes;
    int         write_bytes;
    int         fd_file;
    sockaddr_un name;

    init();

    conn_sock = create_connection_socket();
    if (conn_sock == ERROR_SERVER_DOMAIN_SOCKET_CREATE) {
        graceful_exit("error socket creation\n", ERROR_SERVER_DOMAIN_SOCKET_CREATE);
    }

    ret = bind_connection_socket(conn_sock, &name);
    if ( ret == ERROR_SERVER_BIND_DOMAIN_SOCKET ) {
        graceful_exit("error socket binding\n", ERROR_SERVER_BIND_DOMAIN_SOCKET);
    }

    ret = listen(conn_sock, SERVER_MAX_QUEUE);
    if ( ret == -1 ) {
        graceful_exit("error listen connection\n", ERROR_SERVER_LISTEN_CONNECTION);
    }

    epoll_fd = create_epoll_fd();
    if ( epoll_fd == ERROR_SERVER_EPOLL_FD_CREATE ) {
        graceful_exit("error create epoll fd\n", ERROR_SERVER_EPOLL_FD_CREATE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = conn_sock;
    if( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &ev) == -1 ) {
        log_error(&log_ctx, "error epollctl");
        graceful_exit("error epollctl\n", ERROR_SERVER_EPOLL_CTL);
    }

    while(1) {

        no_fds = epoll_wait(epoll_fd, all_events, MAX_CONNECTIONS, -1);
        if ( no_fds == -1 ) {
            log_error(&log_ctx, "epoll wait error");
            graceful_exit("err epoll wait\n", ERROR_SERVER_EPOLL_WAIT);
        }

        for( int i=0; i< no_fds; i++ ) {
            event_fd = all_events[i].data.fd;

            // events on the conn socket
            if ( event_fd == conn_sock ) {
                while ( (client_fd = accept(conn_sock, NULL, NULL)) != -1 ) {

                    idx = find_next_available_conn_idx();
                    if ( idx < 0 ) {
                        log_error(&log_ctx, "cannot accept any new connections\n");
                        close(client_fd);
                        break;
                    }
    
                    connections[idx] = client_fd;
                    set_nonblocking(client_fd);
    
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_fd;
    
    
                    if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1 ) {
                        log_error(&log_ctx, "epoll_ctl error\n");
                        close(client_fd);
                    }
                }
            } else if ( all_events[i].events & EPOLLIN ){

                // events on any of the other sockets
                read_bytes = read(event_fd, buffer, sizeof(buffer));
                if ( read_bytes == 0 ) {
                    // client hug up?
                    log_error(&log_ctx, "client disconnected\n");
                    perror("client disconnected\n");
                    close(event_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
                } else if ( read_bytes == -1 ) {
                    log_error(&log_ctx, "client error\n");
                    perror("client error\n");
                    close(event_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
                } else {
                    buffer[read_bytes] = 0;

                    if ( strncmp(buffer, "GETFD", 5)  == 0 ) {
                        fd_file = open("./testfile.txt", O_RDONLY);
                        if( fd_file < 0 ) {
                            log_error(&log_ctx, "file open error\n");
                            perror("file open error\n");
                        } else {
                            log_info(&log_ctx, "sending file descriptor for testfile.txt to client");
                            if ( send_fd(event_fd, fd_file) < 0 ) {
                                log_error(&log_ctx, "error sending file descriptor\n");
                                perror("error sending file descriptor\n");
                                close(fd_file);
                                fflush(stdout);
                                fflush(stderr);
                                strcpy(buffer, "err reading\n");
                            } else {
                                close(fd_file);
                                continue;
                            }
                        }
                    } else if ( strncmp(buffer, "EXIT", 4) == 0 ) {
                        close(event_fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
                    }

                    write_bytes = write(event_fd, buffer, read_bytes);
                    if ( write_bytes == -1 ) {
                        log_error(&log_ctx, "write error\n");
                        perror("write\n");
                        close(event_fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
                    }

                    printf("client fd %d> %s\n", event_fd, buffer);
                    fflush(stdout);
                    fflush(stderr);
                }
            }

        }
    }

    graceful_exit("shutting down\n", 0);
}

