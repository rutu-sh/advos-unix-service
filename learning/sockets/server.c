#include<sys/socket.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>

#include "connection.h"


int main() {
    int                conn_sock;
    int                data_sock;
    int                ret;
    int                result;
    char               buffer[BUFFER_SIZE];
    ssize_t            r,w;
    struct sockaddr_un name;

    conn_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( conn_sock == -1 ) {
        perror("socket creation\n");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_NAME);

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = bind( conn_sock, (const struct sockaddr*) &name, sizeof(name) );
    if ( ret == -1 ) {
        perror("bind\n");
        exit(EXIT_FAILURE);
    }

    ret = listen(conn_sock, 20);
    if( ret == -1 ) {
        perror("listen\n");
        exit(EXIT_FAILURE);
    }


    for(;;) {
        data_sock = accept(conn_sock, NULL, NULL);
        printf("accepted conn");
        if ( data_sock == -1 ){
            perror("accept\n");
            goto close_d;
        }

        for(;;){
            // memset(buffer, 0, sizeof(buffer));
            r = read(data_sock, buffer, sizeof(buffer));
            if ( r == -1 ) {
                perror("read\n");
                goto close_d;
            }

            buffer[sizeof(buffer) - 1] = 0;

            w = write(data_sock, buffer, sizeof(buffer));
            if( w == -1 ) {
                perror("write\n");
                goto close_d;
            }

            printf("\nclient> %s", buffer);
            fflush(stdout);
        }

close_d:
        printf("\nclosing data fd");
        close(data_sock);
    }

close_c:
    printf("\nclosing conn fd");
    close(conn_sock);
    return 0;
}