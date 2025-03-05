#ifndef AUTH_H
#define AUTH_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/resource.h>

#define INVALID_UID ((uid_t)UINT_MAX)   
#define ALLOWED_UID 1000 

struct ucred {  
    pid_t pid;
    uid_t uid;
    gid_t gid;
};

struct ucred get_client_credentials(int client_fd);
int is_client_authorized(int client_fd);

#endif // AUTH_H
