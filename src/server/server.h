#ifndef UNIX_SERVER_H
#define UNIX_SERVER_H

#include <sys/epoll.h>
#include <sys/un.h>

#include "common/logger.h"
#include "common/connection.h"

#define MAX_SERVICES 16
#define MAX_REQUESTS 16

typedef struct epoll_event epoll_event;
typedef struct sockaddr_un sockaddr_un;
typedef struct sockaddr sockaddr;
typedef struct LogContext LogContext;

extern int connections[MAX_CONNECTIONS];
extern char buffer[BUFFER_SIZE];
extern LogContext log_ctx;
extern epoll_event ev;
extern epoll_event all_events[MAX_CONNECTIONS];
extern int conn_sock;

/*
    Sends CLOSE_CONNECTION to the clients and closes all file descriptors
*/
void graceful_exit(char* msg, int err_code);

/*
    Returns the domain socket.
*/
int create_connection_socket();

/*
    Bind connection socket 
*/
int bind_connection_socket(int conn_sock, sockaddr_un* name);

/*
    Sets an open socket to non-blocking
*/
void set_nonblocking(int fd);

/*
    Create epoll_fd
*/
int create_epoll_fd();

/*
    Returns the next available file descriptor from the connections array
*/
int find_next_available_conn_idx();

/*
    Initialize the server
*/
void init();


#endif // UNIX_SERVER_H