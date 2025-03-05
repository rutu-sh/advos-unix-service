#include "common/logger.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>


const char* test_name = "common_01_test_logger";


int test_std_logger_init() {
    const char* ut_name = "test_std_logger_init";
    struct LogContext log_ctx;
    get_std_logger(&log_ctx);

    assert(log_ctx.isStdLogger == 1);
    assert(log_ctx.outfd == fileno(stdout));
    assert(log_ctx.errfd == fileno(stderr));
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", ut_name, test_name);
    return 0;
}

int test_std_log_info() {
    char buf[30];
    const char* ut_name = "test_std_log_info";
    struct LogContext log_ctx;
    int read_bytes; 
    get_std_logger(&log_ctx);
    log_ctx.isStdLogger = 0;

    int out_pipe[2];
    pipe(out_pipe);
    
    log_ctx.outfd = out_pipe[1];
    log_ctx.errfd = out_pipe[1];

    log_info(&log_ctx, "hello");
    read_bytes = read(out_pipe[0], buf, sizeof(buf));
    buf[read_bytes] = 0;
    assert(strcmp(buf, "[info] hello") == 0);

    log_error(&log_ctx, "hello");
    read_bytes = read(out_pipe[0], buf, sizeof(buf));
    buf[read_bytes] = 0;
    assert(strcmp(buf, "[error] hello") == 0);

    log_debug(&log_ctx, "hello");
    read_bytes = read(out_pipe[0], buf, sizeof(buf));
    buf[read_bytes] = 0;
    assert(strcmp(buf, "[debug] hello") == 0);

    log_warn(&log_ctx, "hello");
    read_bytes = read(out_pipe[0], buf, sizeof(buf));
    buf[read_bytes] = 0;
    assert(strcmp(buf, "[warning] hello") == 0);


    close(out_pipe[0]);
    close(out_pipe[1]);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", ut_name, ut_name);
    return 0;
}



int main() {
    test_std_logger_init();
    test_std_log_info();
}