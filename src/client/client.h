#ifndef UNIX_CLIENT_H
#define UNIX_CLIENT_H

#include "common/connection.h"
#include "common/logger.h"


typedef struct sockaddr_un sockaddr_un;
typedef struct sockaddr sockaddr;
typedef struct LogContext LogContext;

int data_sock;
LogContext log_ctx;

/*
    Initialize the server
*/
void init();

/*

*/
void graceful_exit(char* msg, int err_code);

/*
    Returns the domain socket.
*/
int create_data_socket();


/*
    Returns the epoll file descriptor.
*/
int create_epoll_fd();

/*
    Returns the resource from the message.
*/
char* get_resource_from_message(const char* mes, const char* prefix);

/*
    Handles the standard input.
*/
int handle_stdin_event(char w_buffer[256], char fr_buffer[256]);

/*
    Handles the data socket.
*/
int handle_datasock_event(char r_buffer[256]);

/*
    Sets the socket to non-blocking.
*/
void set_nonblocking(int fd);



#endif