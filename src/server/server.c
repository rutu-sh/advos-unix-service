#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "server.h"
#include "common/passfd.h"
#include "common/connection.h"
#include "common/errorcodes.h"


void init() {
    
    // init empty clients
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i].client_fd = -1;
        memset(connections[i].resource, 0, sizeof(connections[i].resource));
    }

    get_std_logger(&log_ctx);
    conn_sock = -1;
}


void graceful_exit(char* msg, int err_code) {

    char print_buffer[256];

    log_info(&log_ctx, "graceful exit called\n");

    if ( conn_sock != -1 ) {
        log_info(&log_ctx, "closing data socket\n");
        close(conn_sock);
    }

    sprintf(print_buffer, "exiting - %d: %s", err_code, msg);

    log_error(&log_ctx, print_buffer);
    log_info(&log_ctx, print_buffer);
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

    memset(name, 0, sizeof(*name));
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
        if ( connections[i].client_fd == -1 ) {
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

int do_op(int epoll_fd, int event_fd, char* buffer) {

    printf("buffer and client fd: %s %d\n", buffer, event_fd);

    // PUB <resource>
    if ( strncmp(buffer, "PUB", 3)  == 0 ) {
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (connections[i].client_fd == event_fd) {
                memset(connections[i].resource, 0, sizeof(connections[i].resource));
                strcpy(connections[i].resource, get_resource_from_message(buffer, "PUB"));

                // log_info(&log_ctx, "client published resource\n");
                break;
            }
        }

        log_info(&log_ctx, "client published resource: \n");
        return 0;
    }

    // REQ <resource>
    else if ( strncmp(buffer, "REQ", 3) == 0 ) {
        char *file_needed = get_resource_from_message(buffer, "REQ");
        if (file_needed == NULL) {
            log_error(&log_ctx, "error getting resource from client message\n");
            perror("error reading resource\n");
            return -1;
        }

        // loop through all clients and check if they have the resource
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (connections[i].client_fd != -1) {
                if (strcmp(connections[i].resource, file_needed) == 0) {

                    // found the resource
                    log_info(&log_ctx, "found resource\n");

                    // ask client for resource fd
                    char cl_mess[256] = "REQ_";
                    strcat(cl_mess, connections[i].resource);

                    printf("sending REQ message to client\n");

                    if (write(connections[i].client_fd, cl_mess, sizeof(cl_mess)) < 0) {
                        log_error(&log_ctx, "error sending REQ message to client\n");
                        perror("error sending REQ message to client\n");
                        return -1;
                    }

                    printf("sent REQ message to client\n");

                    int resource_fd = recv_fd(connections[i].client_fd);
                    if (resource_fd < 0) {
                        log_error(&log_ctx, "error receiving resource fd\n");
                        perror("error receiving resource fd\n");
                        return -1;
                    }


                    printf("received resource fd %d\n", resource_fd);

                    // send to client
                    if (send_fd(event_fd, resource_fd) < 0) {
                        log_error(&log_ctx, "error sending resource fd\n");
                        perror("error sending resource fd\n");
                        return -1;
                    }

                    // success
                    log_info(&log_ctx, "sent resource to client\n");
                    close(resource_fd);
                    return 0;
                }
            }
        }

        // resource not found
        log_info(&log_ctx, "resource not found\n");
        send_fd(event_fd, STDIN_FILENO);
        return 0;
    }

    else if ( strncmp(buffer, "exit", 4) == 0 ) {
                        
        log_info(&log_ctx, "closing fd\n");
        close(event_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
        return 0;
    }

    log_error(&log_ctx, "invalid message from client\n");
    perror("invalid message from client\n");
    return -1;
}

char* get_resource_from_message(const char* mes, const char* prefix) {
    if (strncmp(mes, prefix, strlen(prefix)) == 0) {
        char* rest = strchr(mes, '_');
        if (rest != NULL) {
            return rest + 1;
        }
    }
    return NULL;
}