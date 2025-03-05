#include "auth.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>


#define INVALID_UID ((uid_t)-1)  
#define ALLOWED_UID 1000 

struct ucred get_client_credentials(int client_fd) {
    struct ucred creds;

    memset(&creds, 0, sizeof(creds));

    socklen_t len = sizeof(creds);
    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &creds, &len) == -1) {
        syslog(LOG_ERR, "getsockopt failed: %s", strerror(errno));
        creds.uid = INVALID_UID;  }
    return creds;
}

int is_client_authorized(int client_fd) {
    struct ucred creds = get_client_credentials(client_fd);
    if (creds.uid == INVALID_UID) {
        syslog(LOG_WARNING, "Failed to retrieve client credentials.");
        return 0;
    }
    
    syslog(LOG_INFO, "Client connected: PID=%d, UID=%d, GID=%d", creds.pid, creds.uid, creds.gid);
    
    if (creds.uid != ALLOWED_UID) {
        syslog(LOG_WARNING, "Unauthorized client (UID=%d). Closing connection.", creds.uid);
        return 0;
    }

    syslog(LOG_INFO, "Authorized client.");
    return 1;
}
