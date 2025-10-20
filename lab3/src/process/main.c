// little sample program for the client to run
#include <stdio.h>

// write to a file the argc and argv values
int main(int argc, char* argv[]) {

    FILE* file = fopen("process_output.txt", "w");
    if (file == NULL) {
        return 1;
    }

    fprintf(file, "argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        fprintf(file, "argv[%d]: %s\n", i, argv[i]);
    }

    return 0;
}