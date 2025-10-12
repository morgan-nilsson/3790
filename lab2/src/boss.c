#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "header.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "USAGE:%s <num_workers> <max_number>\n", argv[0]);
        return 1;
    }

    // ensure the data/ dir exists
    struct stat st = {0};
    if (stat("data", &st) == -1) {
        mkdir("data", 0700);
        if (stat("data", &st) == -1) {
            fprintf(stderr, "Attempted to create the data/ directory but failed\n");
            return 1;
        }
    }

    // parse worker_count
    char *endptr;
    long w = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || w <= 0) {
        fprintf(stderr, "Invalid number of workers: %s\n", argv[1]);
        return 1;
    }
    const int worker_count = (int) w;

    // parse max_prime
    w = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || w <= 0) {
        fprintf(stderr, "Invalid max number: %s\n", argv[2]);
        return 1;
    }
    const int max_prime = (int)w;

    int len;

    int base = max_prime / worker_count;
    int rem = max_prime % worker_count;
    int current = 1; // Start of the first range

    for (int i = 0; i < worker_count; ++i) {
        int start_value = current;
        int end_value = start_value + base - 1;

        if (i < rem) {
            end_value++; // Distribute the remainder among first `rem` workers
        }

        // Write start
        char start[BUFFER_SIZE];
        len = snprintf(start, BUFFER_SIZE, "%d", start_value);
        if (len >= BUFFER_SIZE || len < 0) {
            fprintf(stderr, "snprintf error for start\n");
            return 1;
        }

        // Write end
        char end[BUFFER_SIZE];
        len = snprintf(end, BUFFER_SIZE, "%d", end_value);
        if (len >= BUFFER_SIZE || len < 0) {
            fprintf(stderr, "snprintf error for end\n");
            return 1;
        }

        current = end_value + 1; // Start next range after this one

        // make child process
        pid_t pid = fork();
        if (pid == -1) {

            fprintf(stderr, "Fork failed");
            return 1;

        // is child
        } else if (!pid) {

            // ensure WORKER_EXE exists
            if (stat(WORKER_EXE_PATH, &st) == -1) {
                fprintf(stderr, "executable worker %s doesn't exist\n", WORKER_EXE_PATH);
                return 1;
            }

            // ensure WORKER_EXE is a regular file
            if (!S_ISREG(st.st_mode)) {
                fprintf(stderr, "executable worker %s is not a regular file\n", WORKER_EXE_PATH);
                return 1;
            }

            // ensure execute permission for owner, group or others
            if (!(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                fprintf(stderr, "%s is not executable\n", WORKER_EXE_PATH);
                return 1;
            }

            // execute WORKER_EXE
            execl(WORKER_EXE_PATH, WORKER_EXE_PATH, start, end, NULL);

            // error if here
            fprintf(stderr, "Exec failed");
            return 1;
        }
    }

    for (int i = 0; i < worker_count; i++) {
        wait(NULL);
    }

    // clear file
    FILE *fw = fopen(FILE_OUTPUT_PATH, "w");
    if (fw == NULL) {
        fprintf(stderr, "Failed to open file %s to clear it\n", FILE_OUTPUT_PATH);
        return 1;
    }
    fclose(fw);

    FILE *f = fopen(FILE_OUTPUT_PATH, "a");
    if (f == NULL) {
        fprintf(stderr, "Failed to open file %s in append mode\n", FILE_OUTPUT_PATH);
        return 1;
    }

    char data_filename[BUFFER_SIZE];
    char buf[BUFFER_SIZE];

    current = 1;

    // collect all results
    for (int i = 0; i < worker_count; i++) {

        int start_value = current;
        int end_value = start_value + base - 1;

        if (i < rem) {
            end_value++; // Distribute the remainder among first `rem` workers
        }

        current = end_value + 1; // Start next range after this one

        // determine data file filename
        len = snprintf(data_filename, BUFFER_SIZE, "data/%d-%d.txt", start_value, end_value);
        if (len >= BUFFER_SIZE) {
            fprintf(stderr, "The size of the given numbers was too large. Consider increasing BUFFER_SIZE\n");
            return 1;
        } else if (len < 0) {
            fprintf(stderr, "There was an snprintf error\n");
            return 1;
        }

        // read from data file
        FILE *data_file = fopen(data_filename, "r");
        if (data_file == NULL) {
            fprintf(stderr, "Failed to read from %s\n", data_filename);
            return 1;
        }

        // read data file to output file
        while (fgets(buf, BUFFER_SIZE, data_file)) {
            fprintf(f, "%s", buf);
        }

        fclose(data_file);
    }

    return 0;
}