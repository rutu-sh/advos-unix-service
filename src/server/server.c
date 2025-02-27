#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "connection.h"
#include "common/errorcodes.h"

void init() {
    memset(connections, -1, sizeof(connections));
    get_std_logger(&log_ctx);
}


void graceful_exit(char* msg, int err_code) {
    log_info(&log_ctx, "graceful exit called");

    log_info(&log_ctx, "closing connections");
    for(int i=0; i< MAX_CONNECTIONS; i++){
        if ( connections[i] == -1 ) {
            close(connections[i]);
        }
    }

    log_info(&log_ctx, "exiting");
    exit(err_code);
}


int create_domain_socket(){
    int conn_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( conn_sock == -1 ) {
        log_info(&log_ctx, "error creating domain socket");
        perror("error creating domain socket");
        graceful_exit("closing all file descriptors", ERROR_SERVER_DOMAIN_SOCKET_CREATION);
    }

    unlink(SOCKET_NAME);

    

}