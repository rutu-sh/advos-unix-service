#include "auth.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>  
#include <unistd.h>
#include <stdio.h>

struct ucred get_client_credentials(int client_fd) {
    struct ucred creds;
    socklen_t len = sizeof(creds);
    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &creds, &len) == -1) {
        perror("getsockopt");
        creds.uid = -1;     }
    return creds;
}

int is_client_authorized(int client_fd) {
    struct ucred creds = get_client_credentials(client_fd);
    if (creds.uid == -1) {
        return 0; 
    }
    
    printf("Client connected: PID=%d, UID=%d, GID=%d\n", creds.pid, creds.uid, creds.gid);
    
    if (creds.uid != 1000) {
        printf("Unauthorized client (UID=%d). Closing connection.\n", creds.uid);
        return 0;
    }
    
    printf("Authorized client.\n");
    return 1;
}
