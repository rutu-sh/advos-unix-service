#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <assert.h>

#include "client/client.h"
#include "common/passfd.h"
#include "common/errorcodes.h"

LogContext log_ctx;
int data_sock;

const char* test_name = "client_01_test_utils";
int log_file;

void _replace_log_fds(){
    log_ctx.errfd = log_file;
    log_ctx.outfd = log_file;
    log_ctx.isStdLogger = 0;
}

void _reset_to_defaults(){
    init();
    _replace_log_fds();
}

void test_init() {
    char* ut_name = "test_init";
    init();
    assert(data_sock == -1);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);
}

void test_create_data_socket() {
    char* ut_name = "test_create_data_socket";
    int sock = create_data_socket();
    assert(sock != ERROR_CLIENT_DATA_SOCKET_CREATE);
    close(sock);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);
}

void test_create_epoll_fd() {
    char* ut_name = "test_create_epoll_fd";
    int epfd = create_epoll_fd();
    assert(epfd != ERROR_SERVER_EPOLL_FD_CREATE);
    close(epfd);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);
}

void test_set_nonblocking() {
    char* ut_name = "test_set_nonblocking";
    int sock = create_data_socket();
    set_nonblocking(sock);
    int flags = fcntl(sock, F_GETFL, 0);
    close(sock);
    assert( (flags & O_NONBLOCK) != 0 );
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);
}


int main() {
    log_file = open("client_test.log", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    
    _reset_to_defaults();
    test_init();

    _reset_to_defaults();
    test_create_data_socket();
    
    _reset_to_defaults();
    test_create_epoll_fd();

    _reset_to_defaults();
    test_set_nonblocking();
    
    close(log_file);
    return 0;
}