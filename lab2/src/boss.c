#include "small.h"

int main(int argc, char *argv[]) {
    GOOD_ARGS(argc, "<num_workers> <range_of_primes>");

    fclose(fopen("primes.txt", "w")); // clear the file, ignore errors

    for (int i = 0, ws = atoi(argv[1]), r = atoi(argv[2]); i < ws; ++i) {

        char start[256]; snprintf(start, 256, "%d", (i * r / ws) + 1);
        char end[256]; snprintf(end, 256, "%d", ((i+1) * r / ws) + 1);
        if (!fork()) {EXEC("./build/worker", "./worker", start, end, NULL);}
    }
    //wait(NULL); don't wait, there is nothing to wait for. This line might be needed on some systems
}