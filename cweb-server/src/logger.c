#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "../include/logger.h"

static FILE *log_fp = NULL;
static int use_file = 0;

static const char *level_str(LogLevel level)
{
    switch (level)
    {
    case LOG_INFO:
        return "INFO ";
    case LOG_WARN:
        return "WARN ";
    case LOG_ERROR:
        return "ERROR";
    default:
        return "INFO ";
    }
}

static const char *level_color(LogLevel level)
{
    switch (level)
    {
    case LOG_INFO:
        return "\033[36m"; // cyan
    case LOG_WARN:
        return "\033[33m"; // yellow
    case LOG_ERROR:
        return "\033[31m"; // red
    default:
        return "\033[0m";
    }
}

void logger_init(const char *filepath)
{
    if (filepath)
    {
        log_fp = fopen(filepath, "a");
        use_file = (log_fp != NULL);
    }
}

void logger_log(LogLevel level, const char *fmt, ...)
{
    /* Timestamp */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);

    /* Format message */
    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    /* Console (with color) */
    printf("%s[%s]%s [%s] %s\n",
           level_color(level), level_str(level), "\033[0m", ts, msg);
    fflush(stdout);

    /* File (no color codes) */
    if (use_file && log_fp)
    {
        fprintf(log_fp, "[%s] [%s] %s\n", level_str(level), ts, msg);
        fflush(log_fp);
    }
}

void logger_close(void)
{
    if (log_fp)
    {
        fclose(log_fp);
        log_fp = NULL;
    }
}