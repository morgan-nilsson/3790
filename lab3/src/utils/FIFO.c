#include "utils/FIFO.h"
#include "utils/logger.h"

void ensure_fifo_exists(const char* fifo_path, mode_t mode) {

    struct stat st;
    // Check if the path exists
    if (stat(fifo_path, &st) == 0) {
        // Path exists, check if it's a FIFO
        if (S_ISFIFO(st.st_mode)) {
            printf("FIFO already exists at: %s\n", fifo_path);
            return;
        } else {
            fprintf(stderr, "Error: File exists and is not a FIFO: %s\n", fifo_path);
            exit(EXIT_FAILURE);
        }
    }
    // Path doesn't exist, create the FIFO
    if (mkfifo(fifo_path, mode) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

}