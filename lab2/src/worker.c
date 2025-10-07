#include "small.h"

static inline int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i*i <= n; i++)
        if (n % i == 0) return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    GOOD_ARGS(argc, "<start_range> <end_range>");
    
    FILE *fp = fopen("primes.txt", "a");

    for (int i = atoi(argv[1]), end = atoi(argv[2]); i < end; i++) {
        if (is_prime(i)) fprintf(fp, "%d\n", i); fflush(fp);
    }
    // fclose(fp); // the os will do this anyway
}
