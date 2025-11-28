#include "bankers.h"
#include <stdlib.h>
#include <stdio.h>

void initBankerFromFile(Banker* banker, const char* filename);

int main(void) {

    Banker banker;
    initBankerFromFile(&banker, "bankerfile.bnk");

    while (1) {
        printBankers(&banker);

        int processID;
        printf("Enter process ID (-1 to exit): ");
        scanf("%d", &processID);
        if (processID == -1) {
            break;
        }
        if (processID < 0 || processID >= banker.numProcesses) {
            printf("Invalid process ID.\n");
            continue;
        }

        int* request = (int*)malloc(sizeof(int) * banker.numResources);
        printf("Enter request for process %d: ", processID);
        for (int j = 0; j < banker.numResources; j++) {
            scanf("%d", &request[j]);
        }

        RequestResult result = make_request(&banker, processID, request);
        if (result == REQUEST_GRANTED) {
            printf("Request granted.\n");
        } else if (result == REQUEST_DENIED_NEEDS_EXCEEDED) {
            printf("Request denied: exceeds process needs.\n");
        } else if (result == REQUEST_DENIED_NOT_ENOUGH_AVAILABLE) {
            printf("Request denied: not enough available resources.\n");
        } else if (result == REQUEST_DENIED_UNSAFE_STATE) {
            printf("Request denied: would lead to unsafe state.\n");
        } else {
            printf("Unknown result.\n");
        }

        free(request);
    }

    printBankers(&banker);
    freeBanker(&banker);

    return 0;
}

void initBankerFromFile(Banker* banker, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(1);
    }

    int numProcesses;
    int numResources;

    fscanf(file, "%d", &numProcesses);
    fscanf(file, "%d", &numResources);

    initBanker(banker, numProcesses, numResources);

    for (int i = 0; i < banker->numProcesses; i++) {
        for (int j = 0; j < banker->numResources; j++) {
            int maxVal;
            fscanf(file, "%d", &maxVal);
            setElementMatN(&banker->max, i, j, maxVal);
            setElementMatN(&banker->need, i, j, maxVal); // initially need = max
            setElementMatN(&banker->allocation, i, j, 0); // initially allocation = 0
        }
    }

    for (int j = 0; j < banker->numResources; j++) {
        fscanf(file, "%d", &banker->available[j]);
    }
}