#include "utils/logger.h"

#include <stdlib.h>

Logger_t _my_logger = {0};

void create_logger(Logger_t* logger, const char* path, enum LogLevel level) {
    logger->log_file_path = strdup(path);
    if (logger->log_file_path == NULL) {
        perror("Failed to allocate memory for log file path");
        exit(1);
    }
    logger->log_level = level;
    logger->log_file = fopen(path, "a");
    if (logger->log_file == NULL) {
        perror("Failed to open log file");
        exit(1);
    }
}

void log_message(Logger_t* logger, enum LogLevel level, const char* message) {
    if (level >= logger->log_level && logger->log_file != NULL) {
        fprintf(logger->log_file, "%s\n", message);
        fflush(logger->log_file);
    }
}

void close_logger(Logger_t* logger) {
    if (logger->log_file != NULL) {
        fclose(logger->log_file);
    }
    free(logger->log_file_path);
}