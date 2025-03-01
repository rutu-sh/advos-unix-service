#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>

#include "server.h"
#include "connection.h"
#include "common/errorcodes.h"


void init() {
    memset(connections, -1, sizeof(connections));
    get_std_logger(&log_ctx);
}


void graceful_exit(char* msg, int err_code) {
    log_info(&log_ctx, "graceful exit called\n");

    log_info(&log_ctx, "closing connections\n");
    for(int i=0; i< MAX_CONNECTIONS; i++){
        if ( connections[i] != -1 ) {
            close(connections[i]);
        }
    }

    close(conn_sock);
    unlink(SOCKET_NAME);
    log_info(&log_ctx, "exiting\n");
    exit(err_code);
}


int create_connection_socket(){

    log_info(&log_ctx, "creating domain socket\n");
    int conn_sock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if ( conn_sock == -1 ) {
        log_info(&log_ctx, "error creating domain socket\n");
        perror("error creating domain socket\n");
        return ERROR_SERVER_DOMAIN_SOCKET_CREATE;
    }
    unlink(SOCKET_NAME);    

    return conn_sock;
}


int bind_connection_socket(int conn_sock, sockaddr_un* name){

    int ret = -1;

    if ( name == NULL || sizeof(*name) == 0 ) {
        return ERROR_SERVER_BIND_DOMAIN_SOCKET;
    }
    log_info(&log_ctx, "binding socket to address\n");

    memset(&name, 0, sizeof(*name));
    name->sun_family = AF_UNIX;
    strncpy(name->sun_path, SOCKET_NAME, sizeof(name->sun_path) - 1);

    ret = bind(conn_sock, (const sockaddr*)name, sizeof(*name));
    if( ret == -1 ) {
        log_error(&log_ctx, "error binding socket to address\n");
        return ERROR_SERVER_BIND_DOMAIN_SOCKET;
    }

    log_info(&log_ctx, "bind successful\n");
    return 0;
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


int find_next_available_conn_idx() {
    for(int i=0; i< MAX_CONNECTIONS; i++){
        if ( connections[i] == -1 ) {
            return i;
        }
    }
    return -1;
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
