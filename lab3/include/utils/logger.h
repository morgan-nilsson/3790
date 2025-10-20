#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <string.h>

enum LogLevel {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
};

struct logger {
    char* log_file_path;
    enum LogLevel log_level;
    FILE *log_file;
};
typedef struct logger Logger_t;

extern Logger_t _my_logger;

#define CREATE_LOGGER(path, level) \
    create_logger(&_my_logger, path, level);

/**
 * Creates and initializes a logger that writes to the specified file path with the given log level.
 */
void create_logger(Logger_t* logger, const char* path, enum LogLevel level);

#define LOG_MESSAGE(level, message) \
    log_message(&_my_logger, level, message);

/** 
 * Logs a message at the specified log level.
 */
void log_message(Logger_t* logger, enum LogLevel level, const char* message);

#define CLOSE_LOGGER() \
    close_logger(&_my_logger);

/**
 * Closes the logger and releases associated resources.
 */
void close_logger(Logger_t* logger);

#endif