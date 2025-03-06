#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <assert.h>

#include "server/server.h"
#include "common/passfd.h"
#include "common/errorcodes.h"


client_inst_t connections[MAX_CONNECTIONS];
char buffer[BUFFER_SIZE];
LogContext log_ctx;
epoll_event ev;
epoll_event all_events[MAX_CONNECTIONS];
int conn_sock;

const char* test_name = "server_01_test_utils";
int log_file;


void _replace_log_fds(){
    log_ctx.errfd = log_file;
    log_ctx.outfd = log_file;
    log_ctx.isStdLogger = 0;
}


void _reset_to_defaults(){
    init();
    _replace_log_fds();
    for(int i=0; i< MAX_CONNECTIONS; i++) {
        connections[i].client_fd = -1;
        memset(connections[i].resource, 0, sizeof(connections[i].resource));
    }
}


void test_init() {
    char* ut_name = "test_init";
    init();
    for(int i=0; i< MAX_CONNECTIONS; i++){
        assert(connections[i].client_fd == -1);
    }
    assert(conn_sock == -1);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);       
}


void test_create_connection_socket() {
    char* ut_name = "test_create_connection_socket";
    sockaddr_un name;
    socklen_t name_len = sizeof(name);

    _replace_log_fds();
    int sock = create_connection_socket();
    assert(sock != -1);

    if ( getsockname(sock, (sockaddr*)&name, &name_len) == -1) {
        perror("getsocketname\n");
        close(sock);
        return;
    }

    close(sock);
    unlink(SOCKET_NAME);
    assert(name.sun_family == AF_UNIX);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);       
}


void test_bind_connection_socket() { 
    char* ut_name = "test_bind_connection_socket";
    sockaddr_un name;

    int sock = create_connection_socket();
    int ret = bind_connection_socket(sock, &name);

    // test happy flow
    close(sock);
    assert(ret != ERROR_SERVER_BIND_DOMAIN_SOCKET);
    assert(name.sun_family == AF_UNIX);
    assert(strcmp(name.sun_path, SOCKET_NAME) == 0);

    // test error flow
    unlink(SOCKET_NAME);
    sock = create_connection_socket();
    ret = bind_connection_socket(sock, NULL);
    close(sock);
    assert(ret == ERROR_SERVER_BIND_DOMAIN_SOCKET);

    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name); 
    
}


void test_create_epoll_fd(){
    char* ut_name = "test_create_epoll_fd";
    int efd = create_epoll_fd();
    close(efd);
    assert(efd != ERROR_SERVER_EPOLL_FD_CREATE);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name); 
}


void test_set_nonblocking() {
    char* ut_name = "test_set_nonblocking";
    
    int sock = create_connection_socket();
    set_nonblocking(sock);
    int flags = fcntl(sock, F_GETFL, 0);
    close(sock);
    assert( (flags & O_NONBLOCK) != 0 );
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name); 
}


void test_get_resource_from_message() {
    char* ut_name = "test_get_resource_from_message";
    assert(strcmp(get_resource_from_message("REQ abc.txt", "REQ "), "abc.txt") == 0);
    assert(strcmp(get_resource_from_message("PUB abc.txt", "PUB "), "abc.txt") == 0); 
    assert(get_resource_from_message("REQ_abc.txt", "PUB ") == NULL);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name); 
}


void test_do_op() {
    char* ut_name = "test_do_op";
    
    client_inst_t temp_client;
    temp_client.client_fd = 123; // random fd
    int idx = 3; // random index        
    int efd = create_epoll_fd();
    
    connections[idx] = temp_client;
    strcpy(buffer, "PUB_test.txt");
    int ret = do_op(efd, temp_client.client_fd, "PUB test.txt");
    memset(buffer, 0, sizeof(buffer));
    close(efd);
    
    assert(ret == 0);
    assert(connections[idx].client_fd == temp_client.client_fd);
    assert(strcmp(connections[idx].resource, "test.txt") == 0);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name); 
}




int main() {
    log_file = open("test.log", O_CREAT | O_WRONLY | O_TRUNC);

    test_init();

    _reset_to_defaults();
    test_create_connection_socket();


    _reset_to_defaults();
    test_bind_connection_socket();


    _reset_to_defaults();
    test_create_epoll_fd();

    _reset_to_defaults();
    test_set_nonblocking();

    _reset_to_defaults();
    test_do_op();

    _reset_to_defaults();
    test_get_resource_from_message();
}
