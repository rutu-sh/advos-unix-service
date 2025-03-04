#include "common/logger.h"
#include "common/connection.h"
#include "client/client.h"

#include <stdio.h>
#include <assert.h>

LogContext log_ctx;
int        data_sock;

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