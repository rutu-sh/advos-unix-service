#include "common/passfd.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>

const char* test_name = "common_02_test_passfd";

int test_send_recv_fd() {
    int sockpair_fd[2]; // 0 = child, 1 = parent
    int comm_pipe[2];
    char buf[2];
    char filebuf[30];
    int read_bytes; 

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair_fd) < 0) {
        perror("Error creating socketpair");
        exit(EXIT_FAILURE);
    }

    if (pipe(comm_pipe) < 0) {
        perror("Error creating pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child: sends the fd
        int file_fd = open("tests/test_file.txt", O_RDONLY);
        if (file_fd < 0) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        // Wait for parent to start reading
        if (read(comm_pipe[0], buf, 2) != 2) {
            perror("Error reading from pipe");
            exit(EXIT_FAILURE);
        }

        if (send_fd(sockpair_fd[0], file_fd) < 0) {
            perror("Error sending file descriptor");
            exit(EXIT_FAILURE);           
        }

        close(file_fd);
        exit(EXIT_SUCCESS); 

    } else if (pid > 0) {
        // Parent: receives the fd
        if (write(comm_pipe[1], "OK", 2) != 2) {
            perror("Error communicating via pipe");
            exit(EXIT_FAILURE);
        }

        int fd_recv = recv_fd(sockpair_fd[1]);
        if (fd_recv < 0) {
            perror("Error receiving file descriptor");
            return -1;
        }
        close(sockpair_fd[1]);

        read_bytes = read(fd_recv, filebuf, sizeof(filebuf));
        if (read_bytes < 0) {
            perror("Error reading from passed fd");
            exit(EXIT_FAILURE);
        }

        filebuf[read_bytes] = 0;
        assert(strcmp(filebuf, "hello") == 0 && "File content does not match");
        close(fd_recv);
        return 0;

    } else {
        perror("Error forking process");
        return -1;
    }

    return -1;
}

int main() {
    char* ut_name;
    
    ut_name = "test_send_recv_fd";
    if (test_send_recv_fd() != 0) {
        printf("test:%s:\033[1;33m%s\033[0m:\033[31mFAILED\033[0m\n", ut_name, ut_name);
    } else {
        printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", ut_name, ut_name);       
    }

    return EXIT_SUCCESS;
}
