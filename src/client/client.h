#ifndef UNIX_CLIENT_H
#define UNIX_CLIENT_H

#include "common/connection.h"
#include "common/logger.h"


typedef struct sockaddr_un sockaddr_un;
typedef struct sockaddr sockaddr;
typedef struct LogContext LogContext;

extern int data_sock;
extern LogContext log_ctx;

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




#endif