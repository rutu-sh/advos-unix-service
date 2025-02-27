#ifndef AUTH_H
#define AUTH_H

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

struct ucred get_client_credentials(int client_fd);
int is_client_authorized(int client_fd);

#endif // AUTH_H
