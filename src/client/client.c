#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>

#include "client.h"
#include "common/connection.h"
#include "common/errorcodes.h"


void init() {
    get_std_logger(&log_ctx);
    data_sock = -1;
}


void graceful_exit(char* msg, int err_code) {

    char print_buffer[256];

    log_info(&log_ctx, "graceful exit called\n");

    if ( data_sock != -1 ) {
        log_info(&log_ctx, "closing data socket\n");
        close(data_sock);
    }

    sprintf(print_buffer, "err %d: %s", err_code, msg);

    log_error(&log_ctx, print_buffer);
    log_info(&log_ctx, print_buffer);
    log_info(&log_ctx, "exiting\n");

    exit(err_code);
}


int create_data_socket() {
    log_info(&log_ctx, "creating data socket\n");

    int d = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( d == -1 ) {
        log_info(&log_ctx, "error creating data socket\n");
        perror("error creating data socket\n");
        return ERROR_CLIENT_DATA_SOCKET_CREATE;
    }

    log_info(&log_ctx, "created data socket\n");

    return d;
}



