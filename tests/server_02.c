#include "common/logger.h"
#include "common/connection.h"
#include "server/server.h"

#include <stdio.h>
#include <assert.h>


client_inst_t connections[MAX_CONNECTIONS];
char buffer[BUFFER_SIZE];
LogContext log_ctx;
epoll_event ev;
epoll_event all_events[MAX_CONNECTIONS];
int conn_sock;

int test_std_logger() {
    struct LogContext log_ctx;
    get_std_logger(&log_ctx);

    assert(log_ctx.isStdLogger == 1);
    assert(log_ctx.outfd == fileno(stdout));
    assert(log_ctx.errfd == fileno(stderr));
    printf("PASSED\n");
    return 0;
}


int main() {
    test_std_logger();
}