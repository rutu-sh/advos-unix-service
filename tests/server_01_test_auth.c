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
#include "server/server.h"
#include "server/auth.h"
#include <assert.h>    
#include <fcntl.h>     
#include <unistd.h>  


client_inst_t connections[MAX_CONNECTIONS];
char buffer[BUFFER_SIZE];
LogContext log_ctx;
epoll_event ev;
epoll_event all_events[MAX_CONNECTIONS];
int conn_sock;


const char* test_name = "server_01_test_auth";
int log_file;

void _replace_log_fds(){
    log_ctx.errfd = log_file;
    log_ctx.outfd = log_file;
    log_ctx.isStdLogger = 0;
}


void test_get_client_credentials_invalid() {
    char* ut_name = "test_get_client_credentials_invalid";
    int fd = -1; // Invalid socket
    
    struct ucred test = get_client_credentials(fd);
    assert(test.uid == (uid_t)-1);
    
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);
}

int main() {
    log_file = open("auth_test.log", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    _replace_log_fds();

    test_get_client_credentials_invalid();
    
    close(log_file);
    return 0;
}
