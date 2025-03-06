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
    char*       fname;
    int         read_bytes;
    int         write_bytes;
    int         received_fd;
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
        nfds  = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if ( nfds == -1 ) {
            log_error(&log_ctx, "error epoll_wait\n");
            graceful_exit("error epoll_wait\n", ERROR_CLIENT_EPOLL_CTL);
        }

        for (int i = 0; i < nfds; i++) {
            printf("event on fd %d\n", events[i].data.fd);

        if ( events[i].data.fd == STDIN_FILENO ) {
            // printf("\nclient> ");
            memset(w_buffer, 0, sizeof(w_buffer));
            if(scanf("%s", w_buffer) == -1){
                continue;
                // log_error(&log_ctx, "error reading from stdin\n");
                // graceful_exit("error reading from stdin\n", ERROR_CLIENT_READ_STDIN);
            }

            printf("after scanf, buffer is: %s\n", w_buffer);            
            /*size_t len = strlen(w_buffer);
            if(len > 0 && w_buffer[len-1] == '\n'){
                w_buffer[len-1] = '\0';
            }*/
            if(strncmp(w_buffer, "exit", 4) == 0){
                write_bytes = write(data_sock, "exit", strlen("exit"));
                graceful_exit("exiting\n", 0);
            }
            else if(strncmp(w_buffer, "PUB", 3) == 0){
                fname=get_resource_from_message(w_buffer, "PUB");
                if(strlen(fname)==0){
                    printf("Missing filename for PUB command\n");
                    continue;
                }
                int fd= open(fname, O_RDWR | O_CREAT, 0666);
                if(fd == -1){
                    printf("Error opening file\n");
                    continue;
                }

                //write some nonsense to distinguish the file
                char text[256] = "This is ";
                strcat(text, fname);
                write(fd, text, sizeof(text));
                lseek(fd, 0, SEEK_SET);

                printf("sending message: %s\n", w_buffer);

                write_bytes = write(data_sock, w_buffer, sizeof(w_buffer));
                if(write_bytes == -1){
                    log_error(&log_ctx, "error writing to socket\n");
                    graceful_exit("error writing to socket\n", 0);
                    close(fd);
                    continue;
                }

                printf("client finished writing to socket after PUB\n");
            }
            else if(strncmp(w_buffer, "REQ", 3) == 0){
                fname=get_resource_from_message(w_buffer, "REQ");
                if(strlen(fname)==0){
                    printf("Missing filename for REQ command\n");
                    continue;
                }
                write_bytes = write(data_sock, w_buffer, sizeof(w_buffer));
                if(write_bytes == -1){
                    log_error(&log_ctx, "error writing to socket\n");
                    graceful_exit("error writing to socket\n", 0);
                    continue;
                }
                
                received_fd = recv_fd(data_sock);
                if(received_fd == -1){
                    log_error(&log_ctx, "error receiving file descriptor\n");
                    continue;
                }
                printf("Received file descriptor %d\n", received_fd);
                while((read_bytes = read(received_fd, fr_buffer, sizeof(fr_buffer))) > 0){
                    fr_buffer[read_bytes] = '\0';
                    printf("%s", fr_buffer);
                }
                if (read_bytes == -1) {
                    log_error(&log_ctx, "error reading from file descriptor\n");
                    continue;
                }
                close(received_fd);
                
            }
            else{
                printf("Invalid command\n");
            }
        }
        // poll for data on the data socket
        else if ( events[i].data.fd == data_sock ) {
            log_info(&log_ctx, "waiting for server to ask for file\n");
                    printf("starting read on data socket\n");
                    read_bytes = read(data_sock, r_buffer, sizeof(r_buffer));
                    printf("read %d bytes\n", read_bytes);
                    if ( read_bytes == -1 ) {
                        log_error(&log_ctx, "error reading from socket\nTry again\n");
                        perror("error reading\n");
                        fflush(stdout);
                        fflush(stderr);
                    }
                    if ( read_bytes == 0 ) {
                        log_error(&log_ctx, "server closed connection\n");
                        graceful_exit("server closed connection\n", 0);
                    }
                    printf("server> %s\n", r_buffer);
                    r_buffer[read_bytes] = '\0';

                    if(strncmp(r_buffer, "REQ", 3) == 0){
                        char *filename;
                        filename = get_resource_from_message(r_buffer, "REQ");
                        // printf("filename: %s\n", filename);
                        int fd_to_send = open(filename, O_RDWR | O_CREAT, 0666);
                        printf("opened file %s\n", filename);
                        if(fd_to_send == -1){
                            log_error(&log_ctx, "error opening file\n");
                            continue;
                        }
                        else{
                            if(send_fd(data_sock, fd_to_send) == -1){
                                log_error(&log_ctx, "error sending file descriptor\n");
                            }
                        }
                        close(fd_to_send);
                    }
                    else{
                        printf("\nserver> %s\n", r_buffer);
                    }
                    
                }
        

    }

    }
    // setup signal handler for this 
    graceful_exit("exiting\n", 0);    
}