#include "auth.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>
#include <time.h>

#define INVALID_UID ((uid_t)-1U)   
#define ALLOWED_UID 1000 
#define MAX_CONNECTIONS 10  

// For tracking resource usage
static int total_attempts = 0;
static int authorized_clients = 0;
static int rejected_clients = 0;

struct ucred get_client_credentials(int client_fd) {
    struct ucred creds;
    memset(&creds, 0, sizeof(creds));
    socklen_t len = sizeof(creds);

    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &creds, &len) == -1) {
        fprintf(stderr, "Error: getsockopt failed: %s\n", strerror(errno));
        creds.uid = INVALID_UID;
    }
    return creds;
}

// Resource usage (CPU & memory)
void track_client_resources(int client_fd) {
    struct ucred creds = get_client_credentials(client_fd);
    if (creds.uid == INVALID_UID) return;

    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        printf("Client PID %d Resource Usage: CPU time (user): %ld sec, Memory: %ld KB\n",
               creds.pid, usage.ru_utime.tv_sec, usage.ru_maxrss);
    } else {
        fprintf(stderr, "Error: Failed to get resource usage for client PID %d: %s\n", creds.pid, strerror(errno));
    }
}

int enforce_rate_limit() {
    if (total_attempts >= MAX_CONNECTIONS) {
        printf("Warning: Too many connection attempts. Limiting access.\n");
        return 0;
    }
    return 1;
}

// Authorization of client
int is_client_authorized(int client_fd) {
    total_attempts++;

    if (!enforce_rate_limit()) {
        return 0;
    }

    struct ucred creds = get_client_credentials(client_fd);
    if (creds.uid == INVALID_UID) {
        printf("Warning: Failed to retrieve client credentials.\n");
        rejected_clients++;
        return 0;
    }

    printf("Client connected: PID=%d, UID=%d, GID=%d\n", creds.pid, creds.uid, creds.gid);

    if (creds.uid != ALLOWED_UID) {
        printf("Warning: Unauthorized client (UID=%d). Closing connection.\n", creds.uid);
        rejected_clients++;
        return 0;
    }

    printf("Authorized client.\n");
    authorized_clients++;

    track_client_resources(client_fd);

    return 1;
}

// Prints authorization statistics
void print_client_usage(int client_fd) {
    struct ucred creds = get_client_credentials(client_fd);
    if (creds.uid == INVALID_UID) return;

    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        printf("Client PID %d disconnected. Final Resource Usage: CPU time (user): %ld sec, Memory: %ld KB\n",
               creds.pid, usage.ru_utime.tv_sec, usage.ru_maxrss);
    } else {
        fprintf(stderr, "Error: Failed to get resource usage for client PID %d: %s\n", creds.pid, strerror(errno));
    }
}
