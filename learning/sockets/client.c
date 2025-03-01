#include<sys/socket.h>
#include<sys/un.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>
#include <fcntl.h>

#include "connection.h"
#include "passfd.h"


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
        
        if (strcmp(w_buffer, "exit_pls") == 0) {
            close(data_sock);
            printf("exiting...\n");
            exit(EXIT_SUCCESS);
        }
        if (strcmp(w_buffer, "GETFD") == 0) {
            w = write(data_sock, w_buffer, strlen(w_buffer));
            if( w == -1 ) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            printf("Request sent, waiting for file descriptor...\n");
            int fd = recv_fd(data_sock);
            if (fd < 0) {
                fprintf(stderr, "recv_fd failed\n");
                continue;
            }
            printf("Received file descriptor: %d\n", fd);
            printf("File content:\n");
            while ((r = read(fd, r_buffer, sizeof(r_buffer)-1)) > 0) {
                r_buffer[r] = '\0';
                printf("%s", r_buffer);
            }
            if (r < 0)
                perror("read");
            close(fd);
            continue;
        }
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