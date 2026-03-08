#ifndef LOGGER_H
#define LOGGER_H

typedef enum
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

void logger_init(const char *filepath);
void logger_log(LogLevel level, const char *fmt, ...);
void logger_close(void);

#define LOG_INFO(...) logger_log(LOG_INFO, __VA_ARGS__)
#define LOG_WARN(...) logger_log(LOG_WARN, __VA_ARGS__)
#define LOG_ERROR(...) logger_log(LOG_ERROR, __VA_ARGS__)

#endif