#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bankers.h"

MatN* createMatN(MatN* matN, int rows, int cols) {
    matN->rows = rows;
    matN->cols = cols;
    matN->data = (int*)malloc(sizeof(int) * rows * cols);
    return matN;
}

void freeMatN(MatN* matN) {
    if (matN == NULL) {
        perror("MatN is NULL");
        return;
    }
    free(matN->data);
    matN->data = NULL;
    matN->rows = 0;
    matN->cols = 0;
}

int getElementMatN(const MatN* matN, int row, int col) {
    if (matN == NULL) {
        perror("MatN is NULL");
        return -1;
    }
    if (row >= matN->rows || col >= matN->cols) {
        perror("Index out of bounds");
        return -1;
    }
    return matN->data[row * matN->cols + col];
}

void setElementMatN(MatN* matN, int row, int col, int value) {
    if (matN == NULL) {
        perror("MatN is NULL");
        return;
    }
    if (row >= matN->rows || col >= matN->cols) {
        perror("Index out of bounds");
        return;
    }
    matN->data[row * matN->cols + col] = value;
}

void printMatN(const MatN* matN) {
    if (matN == NULL) {
        perror("MatN is NULL");
        return;
    }
    for (int i = 0; i < matN->rows; i++) {
        for (int j = 0; j < matN->cols; j++) {
            printf("%d ", getElementMatN(matN, i, j));
        }
        printf("\n");
    }
}

void initBanker(Banker* banker, int numProcesses, int numResources) {
    if (banker == NULL) {
        perror("Banker is NULL");
        return;
    }
    banker->numProcesses = numProcesses;
    banker->numResources = numResources;
    createMatN(&banker->allocation, numProcesses, numResources);
    createMatN(&banker->max, numProcesses, numResources);
    createMatN(&banker->need, numProcesses, numResources);
    banker->available = (int*)malloc(sizeof(int) * numResources);
}

void freeBanker(Banker* banker) {
    if (banker == NULL) {
        perror("Banker is NULL");
        return;
    }
    freeMatN(&banker->allocation);
    freeMatN(&banker->max);
    freeMatN(&banker->need);
    free(banker->available);
    banker->available = NULL;
}

bool isSafe(Banker* banker) {
    if (banker == NULL) {
        perror("Banker is NULL");
        return false;
    }

    int n = banker->numProcesses;
    int m = banker->numResources;

    bool* finish = (bool*)malloc(sizeof(bool) * banker->numProcesses);
    for (int i = 0; i < n; i++) {
        finish[i] = false;
    }

    int* work = (int*)malloc(sizeof(int) * banker->numResources);
    for (int j = 0; j < m; j++) {
        work[j] = banker->available[j];
    }

    int finishedCount = 0;

    while (finishedCount < n) {
        bool progress = false;

        for (int i = 0; i < n; i++) {
            if (!finish[i]) {
                bool canRun = true;

                for (int j = 0; j < m; j++) {
                    if (getElementMatN(&banker->need, i, j) > work[j]) {
                        canRun = false;
                        break;
                    }
                }

                if (canRun) {
                    for (int j = 0; j < m; j++) {
                        work[j] += getElementMatN(&banker->allocation, i, j);
                    }
                    finish[i] = true;
                    finishedCount++;
                    progress = true;
                }
            }
        }

        if (!progress) {
            free(finish);
            free(work);
            return false;   // no process could run == unsafe
        }
    }

    return true; // all processes can finish == safe
}

RequestResult make_request(Banker* banker, int processID, int* request) {
    for (int j = 0; j < banker->numResources; j++) {
        if (request[j] > getElementMatN(&banker->need, processID, j)) {
            return REQUEST_DENIED_NEEDS_EXCEEDED; // request exceeds need or available
        } else if (request[j] > banker->available[j]) {
            return REQUEST_DENIED_NOT_ENOUGH_AVAILABLE; // not enough available
        }
    }

    for (int j = 0; j < banker->numResources; j++) {
        banker->available[j] -= request[j];
        setElementMatN(&banker->allocation, processID, j, getElementMatN(&banker->allocation, processID, j) + request[j]);
        setElementMatN(&banker->need, processID, j, getElementMatN(&banker->need, processID, j) - request[j]);
    }

    if (isSafe(banker)) {
        return REQUEST_GRANTED; // request granted
    } else {
        for (int j = 0; j < banker->numResources; j++) {
            banker->available[j] += request[j];
            setElementMatN(&banker->allocation, processID, j, getElementMatN(&banker->allocation, processID, j) - request[j]);
            setElementMatN(&banker->need, processID, j, getElementMatN(&banker->need, processID, j) + request[j]);
        }
        return REQUEST_DENIED_UNSAFE_STATE; // request denied
    }
}

void printBankers(const Banker* banker) {
    if (banker == NULL) {
        perror("Banker is NULL");
        return;
    }

    printf("Allocation Matrix:\n");
    printMatN(&banker->allocation);
    printf("Max Matrix:\n");
    printMatN(&banker->max);
    printf("Need Matrix:\n");
    printMatN(&banker->need);
    printf("Available Vector:\n");
    for (int j = 0; j < banker->numResources; j++) {
        printf("%d ", banker->available[j]);
    }
    printf("\n");
}