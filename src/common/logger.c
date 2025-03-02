#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


void log_info(struct LogContext* ctx, char* msg) {
    if ( ctx == NULL || msg == NULL ) {
        return;
    }

    int fd = ctx->outfd;
    char *tag = NULL;

    if ( ctx->isStdLogger ) {
        tag = "\033[32m[info] \033[0m";
    } else {
        tag = "[info] ";
    }
    write(fd, tag, strlen(tag));
    write(fd, msg, strlen(msg));
}

void log_debug(struct LogContext* ctx, char* msg) {
    if ( ctx == NULL || msg == NULL ) {
        return;
    }

    int fd = ctx->outfd;
    char* tag = NULL;

    if ( ctx->isStdLogger ) {
        tag = "\033[1;33m[debug] \033[1;0m";
    } else {
        tag = "[debug] ";
    }

    write(fd, tag, strlen(tag));
    write(fd, msg, strlen(msg));
}


void log_warn(struct LogContext* ctx, char* msg) {
    if ( ctx == NULL || msg == NULL ) {
        return;
    }

    int fd = ctx->errfd;
    char* tag = NULL;

    if ( ctx->isStdLogger ) {
        tag = "\033[1;35m[warning] \033[1;0m";
    } else {
        tag = "[warning] ";
    }
    write(fd, tag, strlen(tag));
    write(fd, msg, strlen(msg));
}

void log_error(struct LogContext* ctx, char* msg) {
    if ( ctx == NULL || msg == NULL ) {
        return;
    }

    int fd = ctx->errfd;
    char* tag;

    if ( ctx->isStdLogger ) {
        tag = "\033[1;31m[error] \033[1;0m";
    } else {
        tag = "[error] ";
    }
    write(fd, tag, strlen(tag));
    write(fd, msg, strlen(msg));
}


void get_std_logger(struct LogContext* ctx) {
    ctx->outfd = fileno(stdout);
    ctx->errfd = fileno(stderr);
    ctx->isStdLogger = 1;
}

void get_file_logger(struct LogContext* ctx, char* filename) {
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd < 0 ){
        exit(1);
    }
    ctx->errfd = fd;
    ctx->outfd = fd;
}