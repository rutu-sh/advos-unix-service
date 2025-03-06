#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>

#include "common/passfd.h"
#include "client.h"
#include "common/connection.h"
#include "common/errorcodes.h"

extern int data_sock;

void init() {
    get_std_logger(&log_ctx);
    data_sock = -1;
}


void graceful_exit(char* msg, int err_code) {

    char print_buffer[256];

    log_info(&log_ctx, "graceful exit called\n");

    if ( data_sock != -1 ) {
        log_info(&log_ctx, "closing data socket\n");
        close(data_sock);
    }

    sprintf(print_buffer, "err %d: %s", err_code, msg);

    log_error(&log_ctx, print_buffer);
    log_info(&log_ctx, print_buffer);
    log_info(&log_ctx, "exiting\n");

    exit(err_code);
}


int create_data_socket() {
    log_info(&log_ctx, "creating data socket\n");

    int d = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( d == -1 ) {
        log_info(&log_ctx, "error creating data socket\n");
        perror("error creating data socket\n");
        return ERROR_CLIENT_DATA_SOCKET_CREATE;
    }

    log_info(&log_ctx, "created data socket\n");

    return d;
}

int create_epoll_fd() {
    int epoll_fd = -1;
    epoll_fd = epoll_create1(0);
    if ( epoll_fd == -1 ) {
        log_error(&log_ctx, "error creating epoll fd\n");
        return ERROR_SERVER_EPOLL_FD_CREATE;
    }
    return epoll_fd;
}


char* get_resource_from_message(const char* mes, const char* prefix) {
    char* rest;
    if (strncmp(mes, prefix, strlen(prefix)) == 0) {
        rest = strchr(mes, ' ');
        if (rest == NULL) {
            return NULL;
        }
    }

    // remove annoying newline char
    char *temp = rest + 1;
    while (*temp != '\0') {
        if (*temp == '\n') {
            *temp = '\0';
            break;
        }
        temp++;
    }

    return rest + 1;
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


int handle_stdin_event(char w_buffer[BUFFER_SIZE], char fr_buffer[BUFFER_SIZE]) {
    int write_bytes, read_bytes, received_fd;
    char *fname;

    if (strncmp(w_buffer, "exit", 4) == 0) {
        write_bytes = write(data_sock, "exit", strlen("exit"));
        graceful_exit("exiting\n", 0);
        return -1;
    } else if (strncmp(w_buffer, "PUB", 3) == 0) {
        fname = get_resource_from_message(w_buffer, "PUB");
        if (fname == NULL || strlen(fname) == 0) {
            printf("Missing filename for PUB command\n");
            return -1;
        }
        int fd = open(fname, O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
            printf("Error opening file %s\n", fname);
            return -1;
        }
        char text[256] = "This is ";
        strcat(text, fname);
        write(fd, text, strlen(text));
        lseek(fd, 0, SEEK_SET);

        write_bytes = write(data_sock, w_buffer, strlen(w_buffer));
        if (write_bytes == -1) {
            log_error(&log_ctx, "error writing to socket\n");
            graceful_exit("error writing to socket\n", 0);
            close(fd);
            return -1;
        }
        close(fd);
    } else if (strncmp(w_buffer, "REQ", 3) == 0) {
        fname = get_resource_from_message(w_buffer, "REQ");
        if (fname == NULL || strlen(fname) == 0) {
            printf("Missing filename for REQ command\n");
            return -1;
        }
        write_bytes = write(data_sock, w_buffer, strlen(w_buffer));
        if (write_bytes == -1) {
            log_error(&log_ctx, "error writing to socket\n");
            graceful_exit("error writing to socket\n", 0);
            return -1;
        }
        received_fd = recv_fd(data_sock);
        if (received_fd == -1) {
            log_error(&log_ctx, "error receiving file descriptor\n");
            return -1;
        }
        printf("Received file descriptor %d\n", received_fd);
        while ((read_bytes = read(received_fd, fr_buffer, BUFFER_SIZE)) > 0) {
            fr_buffer[read_bytes] = '\0';
            printf("%s", fr_buffer);
        }
        if (read_bytes == -1) {
            log_error(&log_ctx, "error reading from file descriptor\n");
            return -1;
        }
        close(received_fd);
    } else {
        printf("Invalid command\n");
    }
    return 0;
}


int handle_datasock_event(char r_buffer[BUFFER_SIZE]) {
    int read_bytes, fd_to_send;
    char *filename;

    log_info(&log_ctx, "waiting for server to ask for file\n");
    read_bytes = read(data_sock, r_buffer, BUFFER_SIZE);
    printf("read %d bytes\n", read_bytes);
    if (read_bytes == -1) {
        log_error(&log_ctx, "error reading from socket\nTry again\n");
        perror("error reading\n");
        fflush(stdout);
        fflush(stderr);
        return -1;
    }
    if (read_bytes == 0) {
        log_error(&log_ctx, "server closed connection\n");
        graceful_exit("server closed connection\n", 0);
        return -1;
    }
    r_buffer[read_bytes] = '\0';
    printf("server> %s\n", r_buffer);

    if (strncmp(r_buffer, "REQ", 3) == 0) {
        filename = get_resource_from_message(r_buffer, "REQ");
        if (filename == NULL || strlen(filename) == 0) {
            log_error(&log_ctx, "missing filename in REQ message\n");
            return -1;
        }
        fd_to_send = open(filename, O_RDWR | O_CREAT, 0666);
        printf("opened file %s\n", filename);
        if (fd_to_send == -1) {
            log_error(&log_ctx, "error opening file\n");
            return -1;
        } else {
            if (send_fd(data_sock, fd_to_send) == -1) {
                log_error(&log_ctx, "error sending file descriptor\n");
            }
        }
        close(fd_to_send);
    } else {
        printf("\nserver> %s\n", r_buffer);
    }
    return 0;
}