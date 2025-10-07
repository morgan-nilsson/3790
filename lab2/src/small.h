#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GOOD_ARGS(argc, msg) \
    if (argc != 3) {fprintf(stderr, "Usage: %s %s\n", argv[0], msg); return 1;}

#define EXEC(...) execl(__VA_ARGS__); perror("exec failed"); exit(1);
