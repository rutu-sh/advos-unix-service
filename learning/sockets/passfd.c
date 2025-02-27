//// filepath: /Users/yangboxin/advos-unix-service/learning/sockets/passfd_send.c
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int send_fd(int socket, int fd_to_send) {
    struct msghdr msg = {0};
    char buf[CMSG_SPACE(sizeof(fd_to_send))];
    memset(buf, 0, sizeof(buf));
    
    struct iovec io = { .iov_base = "FD", .iov_len = 2 };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd_to_send));
    
    *((int *) CMSG_DATA(cmsg)) = fd_to_send;
    
    if (sendmsg(socket, &msg, 0) < 0) {
        perror("sendmsg");
        return -1;
    }
    return 0;
}

int recv_fd(int socket) {
    int received_fd = -1;
    struct msghdr msg = {0};
    
    char m_buffer[256];
    struct iovec io = { .iov_base = m_buffer, .iov_len = sizeof(m_buffer) };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    
    char c_buffer[CMSG_SPACE(sizeof(received_fd))];
    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);
    
    if (recvmsg(socket, &msg, 0) < 0) {
        perror("recvmsg");
        return -1;
    }
    
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(received_fd))) {
        if (cmsg->cmsg_level != SOL_SOCKET) {
            fprintf(stderr, "cmsg_level != SOL_SOCKET\n");
            return -1;
        }
        if (cmsg->cmsg_type != SCM_RIGHTS) {
            fprintf(stderr, "cmsg_type != SCM_RIGHTS\n");
            return -1;
        }
        received_fd = *((int *) CMSG_DATA(cmsg));
    } else {
        fprintf(stderr, "No valid passed fd\n");
        return -1;
    }
    
    return received_fd;
}