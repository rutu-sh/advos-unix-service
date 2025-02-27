#ifndef COMMON_LOGGER_H
#define COMMON_LOGGER_H


#define LOG_LEVEL_NONE    0
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARN    2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

#define LOG_LEVEL_ACTIVE LOG_LEVEL_INFO

#include<stdio.h>

struct LogContext {
    int log_level;
    int outfd; // for stdout 
    int errfd; // for stderr
};

void log_info(struct LogContext* ctx, char* msg);
void log_debug(struct LogContext* ctx, char* msg);
void log_warn(struct LogContext* ctx, char* msg);
void log_error(struct LogContext* ctx, char* msg);
void get_std_logger(struct LogContext* ctx);

#endif