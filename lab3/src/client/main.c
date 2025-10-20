#include <stdio.h>
#include "utils/logger.h"
#include "utils/FIFO.h"
#include <assert.h>

#define AUTOMATIC_CLIENT 0

void spin_lock_for_response(char* response, size_t response_size, FILE* fifo) {
    memset(response, 0, response_size);
    while (fgets(response, response_size, fifo) == NULL) {

        unlink(FIFO_S_NAME);

        ensure_fifo_exists(FIFO_S_NAME, FIFO_READWRITE_PERMISSIONS);

        LOG_MESSAGE(LOG_LEVEL_INFO, "FIFO created or already exists.");

        fifo = fopen(FIFO_S_NAME, "r+");
        if (fifo == NULL) {
            LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to open FIFO for reading.");
            fclose(fifo);
            CLOSE_LOGGER();
            return;
        }

    }
    LOG_MESSAGE(LOG_LEVEL_INFO, response);
}

void client_loop_manual(FILE* fifo_r, FILE* fifo_w) {
    char input[1024];
    char response[1024];

    while (1) {

        printf("Enter command (USER or EXEC) or QUIT to exit:\n> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        if (strncmp(input, "QUIT", 4) == 0) {
            break;
        }

        LOG_MESSAGE(LOG_LEVEL_INFO, input);
        fputs(input, fifo_w);
        fflush(fifo_w);

        spin_lock_for_response(response, sizeof(response), fifo_r);
        printf("Server responded: %s", response);
        // check if response contains "Goodbye"
        if (strstr(response, "Goodbye") != NULL) {
            break;
        }
    }
}

int main(void) {
    CREATE_LOGGER("client.log", LOG_LEVEL_DEBUG);

    LOG_MESSAGE(LOG_LEVEL_INFO, "Client is starting...");

    ensure_fifo_exists(FIFO_C_NAME, FIFO_READWRITE_PERMISSIONS);
    ensure_fifo_exists(FIFO_S_NAME, FIFO_READWRITE_PERMISSIONS);


    LOG_MESSAGE(LOG_LEVEL_INFO, "FIFO created or already exists.");

    FILE* fifo_w = fopen(FIFO_C_NAME, "w+");
    if (fifo_w == NULL) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to open FIFO for writing.");
        CLOSE_LOGGER();
        return 1;
    }

    FILE* fifo_r = fopen(FIFO_S_NAME, "r+");
    if (fifo_r == NULL) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to open FIFO for reading.");
        fclose(fifo_w);
        CLOSE_LOGGER();
        return 1;
    }

    LOG_MESSAGE(LOG_LEVEL_INFO, "FIFO opened for writing.");

    client_loop_manual(fifo_r, fifo_w);

    CLOSE_LOGGER();
    return 0;
}
