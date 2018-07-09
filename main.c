
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include "aesni.h"
#include "utilities.h"
#include "file.h"
#include "partition.h"
#include "scheduler.h"

int partition_num = 4; // 4*16mb = 64mb
int tree_size = 65536; // how many block
int block_size = 64; // 4 kilobyte to byte
int Z = 4;
int user_num = 2; // 2 users
int memory_access_circle = 5; // access file every 5 circles
int request_num = 5000; // generate 5000 requests


void Compute_Param(){
    // convert partition_num to a square term
    partition_num = sqrt_int(partition_num) * sqrt_int(partition_num);
    // convert tree_size to a power of 2 term
    tree_size = pow_int(log_int(tree_size));
    printf("Total %d partition, %d tree buckets for each. Each bucket contain %d of %d bytes blocks", partition_num, tree_size, Z, block_size);
}


int main(int argc, const char * argv[]) {

    Compute_Param();
    // key
    uint8_t **public_key = (uint8_t**)malloc(partition_num*sizeof(uint8_t*));
    key_generate(public_key, partition_num);
    uint8_t *aes_key = (uint8_t *)malloc(16);
    memcpy(aes_key, public_key[1], 128);
    
    // e.g
    // permutation[3] = 4 :
    // represent the third block in physical address is the 4th in real address
    
    int *permutation = (int *)malloc(partition_num*sizeof(int));
    rand_permutation(permutation, partition_num);
    
    // in-memory data
    int phase = 1;
    int size = sqrt_int(partition_num);
    Stash *stash = (Stash *)malloc( size * sizeof(Stash *));
    int **pos_map = (int**)malloc( size * sizeof(int*));
    int *in_memory_list = (int*) malloc(size*sizeof(int));
    uint8_t **partitions = (uint8_t**)malloc( size * sizeof(uint8_t*));
    int N = 0;
    
    // data for  scheduler
    ROB * rob_table  = (ROB*) malloc(request_num * sizeof(ROB));
    REQ * request = (REQ*) malloc(memory_access_circle *sizeof(REQ));
    scheduler_init(rob_table, request);
    
    // create files and init them
    FileInit(public_key, aes_key);
    
    // create thread for I/O write or read;
    pthread_t write_thread, read_thread;
    
    // in each access circle, the first 'c' circle is to fetch the missing data in the last circle
    int file_address;
    FILE_INPUT input;
    input.data_buffer = (uint8_t*) malloc(block_size);
    while(1){
        if(N < sqrt_int(partition_num)){
            file_address = schedule(rob_table, request, permutation, in_memory_list);
            if(file_address == -1)
                break;
            input.address = file_address;
            input.phase = phase;
            pthread_create(&read_thread, NULL, File_read, &input);
            
            for(int i=0; i<memory_access_circle; i++){
                Partition_Access(partitions, stash, pos_map, aes_key, permutation, request + i, N);
            }
            pthread_join(read_thread, NULL);
            otp_crypto(input.data_buffer, public_key[permutation[file_address]], (block_size+4)*Z*tree_size + tree_size*4);
            memcpy(pos_map[N], input.data_buffer, tree_size*4);
            memcpy(partitions[N], input.data_buffer + tree_size*4, (block_size+4)*Z*tree_size);
            N++;
        }else{
            Part_Evict(partitions, stash, pos_map, permutation, in_memory_list, public_key, aes_key, phase);
            Fully_Shuffle(permutation, public_key);
            N = 0;
        }
    }
    
    // clean the allocated memory
    free(input.data_buffer);
    scheduler_deinit(rob_table, request);
    free(aes_key);
    for(int i=0; i<partition_num; i++)
        free(public_key[i]);
    return 0;
}



