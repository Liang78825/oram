
#include "scheduler.h"

extern int partition_num;
extern int tree_size;
extern int block_size;
extern int Z;
extern int user_num;
extern int memory_access_circle;
extern int request_num;

void scheduler_init(ROB *rob_table, REQ *request, int* in_memory_list){
    //generate request
    srand(clock());
    int *data_int;
    for(int i=0; i<request_num;i++){
        rob_table[i].finished=-1;
        rob_table[i].partition_address = rand()%partition_num;
        rob_table[i].op = rand()%2;
        rob_table[i].tree_address = rand()%(tree_size - 1);
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
        request[i].op[0] = OP_READ;
        request[i].tree_address = (int*)malloc(sqrt_int(partition_num)*sizeof(int));
        request[i].data = (uint8_t **)malloc(sqrt_int(partition_num)*sizeof(uint8_t*));
        for(int j=0; j<sqrt_int(partition_num); j++){
            request[i].data[j] = (uint8_t*) malloc(block_size);
        }
    }
    for(int i=0; i<partition_num; i++){
        in_memory_list[i] = -1;
    }
}

int schedule(ROB *rob_table, REQ *request, int * permutation, int *in_memory_list, int N){
    srand(clock());
    
    int read_span = 2 * memory_access_circle;
    int j = 0;
    int pos;
    int memory_pos = 0;
    int io_pos = -1;
    
    if(N == 0){
        // first cicle, no memory access
        for(int i=0; i<ROB_SIZE; i++){
            if(rob_table[i].finished == 0){
                return permutation[rob_table[i].partition_address];
            }
        }
    }
    
    // pop the old request
    for(int i=0; j < memory_access_circle && i < ROB_SIZE; i++){
        if(rob_table[i].finished >= 0 ){
            if(rob_table[i].signal != SIG_START){
                if(rob_table[i].op == OP_READ){
                    memcpy(rob_table[i].data, request[rob_table[i].finished].data[rob_table[i].signal], block_size);
                }
                rob_table[i].finished = -2;
                printf("Job %d finished\n", i);
                j++;
            }
        }
    }
    
    // if shuffle period comes, do not sceduler new
    if(N > sqrt_int(partition_num)){
        return 0;
    }
    
    // generate dummy read
    for(int n=0; n<memory_access_circle; n++){
        for(int i=0; i<sqrt_int(partition_num); i++){
            request[n].op[i] = OP_READ;
            request[n].tree_address[i] = rand() % tree_size;
        }
    }
    
    j = 0;
    
    // schedule new request
    for(int i=0; memory_pos <= read_span && j < memory_access_circle; i++){
        
        if(rob_table[i].finished == -1){
            pos =is_in_memory(in_memory_list, permutation[rob_table[i].partition_address]);
            if( pos >= 0 ){
                // already in memory schedule a request
                printf("schedule request:%d\n", i);
                request[j].op[pos] = rob_table[i].op;
                request[j].tree_address[pos] = rob_table[i].tree_address;
                if(rob_table[i].op == OP_WRITE){
                    memcpy(request[j].data[pos], rob_table[i].data, block_size);
                }
                rob_table[i].finished = j;
                rob_table[i].signal = pos;
                j++;
            }
            else{
                // not in memory, schedule a read
                if(io_pos == -1){
                    io_pos = permutation[rob_table[i].partition_address];
                }
            }
            memory_pos++;
        }
    }
    
    // if no IO need, schedule dummy
    if(io_pos == -1){
        int rand_num = rand()%partition_num;
        while(is_in_memory(in_memory_list, rand_num)>=0){
            rand_num = rand()%partition_num;
        }
        io_pos = rand_num;
    }
    // detect if all task is finished
    int is_finish = 0;
    for(int i=0; i< ROB_SIZE; i++){
        if(rob_table[i].finished == -1){
            is_finish++;
            break;
        }
    }
    if(is_finish == 0){
        // all finish
        return -1;
    }
    else{
        return io_pos;
    }
}

int is_in_memory(int * in_memory_list, int address){
    int a;
    for(int i=0; in_memory_list[i]>=0; i++){
        a = in_memory_list[i];
        if(in_memory_list[i] == address)
            return i;
    }
    return -1;
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
