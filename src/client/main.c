#include <stdio.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "client.h"
#include "common/errorcodes.h"

LogContext log_ctx;
int        data_sock;


int main() {
    int         ret;
    char        w_buffer[BUFFER_SIZE];
    char        r_buffer[BUFFER_SIZE];
    int         read_bytes;
    int         write_bytes;
    sockaddr_un name;


    init();


    data_sock = create_data_socket();
    if ( data_sock == ERROR_CLIENT_DATA_SOCKET_CREATE ) {
        log_error(&log_ctx, "error creating data socket\nExiting...\n");
        graceful_exit("err creating data sock\n", ERROR_CLIENT_DATA_SOCKET_CREATE);
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = connect(data_sock, (const sockaddr*)&name, sizeof(name));
    if (ret == -1) {
        perror("connect\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("\nclient> ");
        scanf("%s", w_buffer);

        if ( strcmp(w_buffer, "exit") == 0 ) {
            graceful_exit("closing on request\n", 0);
        }


        write_bytes = write( data_sock, w_buffer, sizeof(w_buffer) );
        if ( write_bytes == -1 ) {
            log_error(&log_ctx, "error writing to socket\nTry again\n");
            perror("error writing\n");
            fflush(stdout);
            fflush(stderr);
        }

        read_bytes = read( data_sock, r_buffer, sizeof(r_buffer) );
        if ( read_bytes == -1 ) {
            log_error(&log_ctx, "error reading from socket\nTry again\n");
            perror("error reading\n");
            fflush(stdout);           
            fflush(stderr);
        }

        printf("\nserver> %s", r_buffer);
        fflush(stdout);
        fflush(stderr);
    }


    // setup signal handler for this 
    graceful_exit("exiting\n", 0);    
}