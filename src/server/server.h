#ifndef UNIX_SERVER_H
#define UNIX_SERVER_H

#include <sys/epoll.h>
#include <common/connection.h>

#include "common/logger.h"

#define MAX_SERVICES 16
#define MAX_REQUESTS 16


int connections[MAX_CONNECTIONS];

char buffer[BUFFER_SIZE];

struct LogContext log_ctx;

typedef struct epoll_event epoll_event;

/*
    Sends CLOSE_CONNECTION to the clients and closes all file descriptors
*/
void graceful_exit();


/*
    Marks a file descriptor to be non blocking by appending O_NONBLOCK to its flags. 
*/
void set_nonblockint(int fd);


/*
    Returns the domain socket.
*/
int create_domain_socket();


/*
    Initialize the server
*/
void init();


#endif // UNIX_SERVER_H