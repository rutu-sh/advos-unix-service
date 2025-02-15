#include<sys/socket.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>

#include "connection.h"


int main() {
    int                data_sock;
    struct sockaddr_un name;
    int                ret;
    char               w_buffer[BUFFER_SIZE];
    char               r_buffer[BUFFER_SIZE]; 
    ssize_t            r, w;

    data_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if( data_sock == -1 ) {
        perror("error data socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1 );

    ret = connect(data_sock, (const struct sockaddr*) &name, sizeof(name));
    if ( ret == -1 ) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    for(;;){
        printf("\nclient> ");
        scanf("%s", w_buffer);
        w = write( data_sock, w_buffer, sizeof(w_buffer) ); 
        if( w == -1 ) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        r = read( data_sock, r_buffer, sizeof(r_buffer) );
        if( r == -1 ) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        printf("\nserver> %s\n", r_buffer);
    }

}