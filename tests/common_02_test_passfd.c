#include "common/passfd.h"
#include "common/connection.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>


#define TEST_SOCKET_NAME "/tmp/test.sock"

int test_send_recv_fd() {
    ssize_t            r;
    struct sockaddr_un name;
    pid_t               pid;
    int                 c_pipe[2];
    char                c_buf[2];
    int                 conn_sock;
    int                 data_sock;
    int                 fd_to_send = -1;
    int                 received_fd;
    char                buffer[100];
    char*               test_buffer = "TEST_MESSAGE";

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, TEST_SOCKET_NAME, sizeof(name.sun_path) - 1);

    pipe(c_pipe);
    pid = fork();
    if ( pid == 0 ) {

        // child process 

        conn_sock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0); 
        bind(conn_sock, (const struct sockaddr*) &name, sizeof(name));
        write(c_pipe[1], "OK", 2);
        listen(conn_sock, 2);

        data_sock = accept(conn_sock, NULL, NULL); 
        fd_to_send = open("./testfile.txt", O_RDONLY);
        send_fd(data_sock, fd_to_send);
        read(c_pipe[0], c_buf, 2);
        close(fd_to_send);
        exit(0);
    } else {
        // parent process - checks for conditions

        read(c_pipe[0], c_buf, 2);
        sleep(1); // don't connect immediately

        data_sock = socket(AF_UNIX, SOCK_STREAM, 0);
        connect(data_sock, (const struct sockaddr*)&name, sizeof(name));
        received_fd = recv_fd(data_sock);

        r = read(received_fd, buffer, sizeof(buffer) - 1);
        buffer[r] = '\0';
        close(received_fd);

        assert( strncmp(buffer, test_buffer, strlen(test_buffer)) == 0 );

        exit(0);

    }

}


int main() {
    test_send_recv_fd();
}