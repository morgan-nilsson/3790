#ifndef FIFOWRAPPER_H
#define FIFOWRAPPER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define FIFO_C_NAME "/tmp/my_c_fifo"
#define FIFO_S_NAME "/tmp/my_s_fifo"

#define FIFO_READWRITE_PERMISSIONS 0666

void ensure_fifo_exists(const char* fifo_path, mode_t mode);

#endif