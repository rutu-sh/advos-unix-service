#ifndef COMMON_PASSFD_H
#define COMMON_PASSFD_H

/*
    send file descriptor to socket
*/
int send_fd(int socket, int fd);


/*
    recv file descriptor from socket
*/
int recv_fd(int socket);

#endif