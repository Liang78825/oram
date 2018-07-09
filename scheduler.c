
#include "scheduler.h"

extern int partition_num;
extern int tree_size;
extern int block_size;
extern int Z;
extern int user_num;
extern int memory_access_circle;
extern int request_num;

void scheduler_init(ROB *rob_table, REQ *request){
    //generate request
    srand(clock());
    int *data_int;
    for(int i=0; i<request_num;i++){
        rob_table[i].finished=0;
        rob_table[i].partition_address = rand()%partition_num;
        rob_table[i].op = rand()%2;
        rob_table[i].tree_address = rand()%(block_size*Z*tree_size/2);
        rob_table[i].user = rand()%user_num;
        rob_table[i].data = (uint8_t*)malloc(block_size);
        rob_table[i].signal = SIG_START;
        data_int = (int*)rob_table[i].data;
        if(rob_table[i].op == OP_READ){ // read
            for(int j=0; j<block_size/4;j++){
                data_int[j] = 0;
            }
        }else{
            for(int j=0; j<block_size/4;j++){
                data_int[j] = rand();
            }
        }
    }
    for(int i=0; i<memory_access_circle; i++){
        request[i].op = (int*)malloc(sqrt_int(partition_num)*sizeof(int));
        request[i].tree_address = (int*)malloc(sqrt_int(partition_num)*sizeof(int));
        request[i].data = (uint8_t **)malloc(sqrt_int(partition_num)*sizeof(uint8_t*));
        for(int j=0; j<sqrt_int(partition_num); j++){
            request[i].data[j] = (uint8_t*) malloc(block_size);
        }
    }

}

int schedule(ROB *rob_table, REQ *request, int * permutation, int *in_memory_list){
    // pop the finished request
    return 1;
}

void scheduler_deinit(ROB *rob_table, REQ *request){
    for(int i=0; i<request_num; i++){
        free(rob_table[i].data);
    }
    free(rob_table);
    for(int i=0; i<memory_access_circle; i++){
        for(int j=0; j<sqrt_int(partition_num); j++){
            free(request[i].data[j]);
        }
        free(request[i].op);
        free(request[i].tree_address);
    }
    free(request);

}
