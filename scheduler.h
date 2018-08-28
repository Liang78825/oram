

#ifndef scheduler_h
#define scheduler_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utilities.h"

#define SIG_START -1
#define SIG_LOADING 1
#define SIG_READY 2

#define OP_READ 1
#define OP_WRITE 0
#define ROB_SIZE 120

typedef struct ROB_t {
    int user;
    int op;
    int signal;
    int finished;
    int partition_address;
    int tree_address;
    uint8_t *data;
} ROB;

typedef struct REQ_t{
    int *op;
    int *tree_address;
    uint8_t **data;
} REQ;


void scheduler_init(ROB *rob_table, REQ *request, int* in_memory_list);

int schedule(ROB *rob_table, REQ *request, int * permutation, int *in_memory_list, int N);

int is_in_memory(int * in_memory_list, int address);

void scheduler_deinit(ROB *rob_table, REQ *request);
#endif /* scheduler_h */
