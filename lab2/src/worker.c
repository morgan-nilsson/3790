#include <stdio.h>
#include <stdlib.h>

#include "header.h"

#define DEBUG 0

static inline int is_prime(int n) {

    if (n < 2) return 0;

    for (int i = 2; i*i <= n; i++)
        if (n % i == 0) return 0;

    return 1;
}

int main(int argc, char *argv[]) {

    if (argc != 3) {

        fprintf(stderr, "USAGE:%s <start_range> <end_range>\n", argv[0]);
        return 1;
    }
    
    int len;
    char buf[BUFFER_SIZE];

    #if DEBUG
    fprintf(stdout, "Finding primes in range %s-%s", argv[1], argv[2]);
    #endif

    len = snprintf(buf, BUFFER_SIZE, "data/%s-%s.txt", argv[1], argv[2]);
    if (len >= BUFFER_SIZE) {
        fprintf(stderr, "The size of the given numbers was too large. Consider increasing BUFFER_SIZE\n");
        return 1;
    } else if (len < 0) {
        fprintf(stderr, "There was an snprintf error\n");
        return 1;
    }

    // clear the data file
    FILE *fw = fopen(buf, "w");
    if (fw == NULL) {
        fprintf(stderr, "Failed to open data file %s to clear it\n", buf);
        return 1;
    }
    fclose(fw);

    FILE *fp = fopen(buf, "a");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open data file %s in apped node\n", buf);
        return 1;
    }
    
    // parse start
    char *endptr;
    long w = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || w <= 0) {
        fprintf(stderr, "Invalid start number: %s\n", argv[1]);
        return 1;
    }
    const int start = (int) w;

    // parse end
    w = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || w <= 0) {
        fprintf(stderr, "Invalid end number: %s\n", argv[2]);
        return 1;
    }
    const int end = (int)w;

    if (start > end) {
        fprintf(stderr, "Start range should be less than or equal to end range\n");
        return 1;
    }

    for (int i = start; i < end + 1; i++) {

        if (is_prime(i)) {

            fprintf(fp, "%d\n", i);

            #if DEBUG
            fprintf(stdout, "%s\n", i);
            #endif
        }
    }
    fclose(fp);

    return 0;
}
