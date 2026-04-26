#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level_t;

#define LOG_DEBUG(fmt, ...) log_msg(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  log_msg(LOG_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  log_msg(LOG_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_msg(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

void log_msg(log_level_t level, const char *file, int line, const char *fmt, ...);


#endif // LOGGER_H
