#include "common/logger.h"

#include <stdio.h>
#include <assert.h>


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