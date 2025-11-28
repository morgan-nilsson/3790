#include <stdbool.h>
#include <stdio.h>

typedef struct MatN {
    int* data;
    int rows;
    int cols;
} MatN;

MatN* createMatN(MatN* matN, int rows, int cols);
void freeMatN(MatN* matN);
int getElementMatN(const MatN* matN, int row, int col);
void setElementMatN(MatN* matN, int row, int col, int value);
void printMatN(const MatN* matN);


typedef struct Banker {
    int numProcesses;
    int numResources;
    MatN allocation;
    MatN max;
    MatN need;
    int* available;
} Banker;

void initBanker(Banker* banker, int numProcesses, int numResources);
void freeBanker(Banker* banker);

typedef enum {
    REQUEST_GRANTED,
    REQUEST_DENIED_NEEDS_EXCEEDED,
    REQUEST_DENIED_NOT_ENOUGH_AVAILABLE,
    REQUEST_DENIED_UNSAFE_STATE,
} RequestResult;
RequestResult make_request(Banker* banker, int processID, int* request);
void printBankers(const Banker* banker);