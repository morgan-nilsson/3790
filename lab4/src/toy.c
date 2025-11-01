#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <char>\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) != 1) {
        printf("Please provide a single character.\n");
        return 1;
    }

    char ch = argv[1][0];

    while (1) {
        putc(ch, stderr);
        usleep(10000);
    }
}