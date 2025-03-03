#include "common/logger.h"

#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

void test_std_logger() {
    struct LogContext log_ctx;
    get_std_logger(&log_ctx);

    assert(log_ctx.isStdLogger == 1);
    assert(log_ctx.outfd == fileno(stdout));
    assert(log_ctx.errfd == fileno(stderr));
    printf("test:test_std_logger:\033[32mPASSED\033[0m\n");
}


int main() {
    test_std_logger();
}