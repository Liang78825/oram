

#include "partition.h"


extern int partition_num;
extern int tree_size;
extern int block_size;
extern int Z;
extern int user_num;
extern int memory_access_circle;
extern int request_num;

void PartInit(uint8_t **partition, Stash *stash, int **pos_map){
    int size = sqrt_int(partition_num);
    partition = (uint8_t**)malloc( size * sizeof(uint8_t*));
    stash = (Stash *)malloc( size * sizeof(Stash *));
    pos_map = (int**)malloc( size * sizeof(int*));
}


void Load_Partition_From_file(uint8_t **partition, Stash *stash, int **pos_map, int pos, uint8_t *key){
    
    int part_size = (block_size + 4)*tree_size * Z;
    int pos_map_size = tree_size * sizeof(int);
    
    partition[pos] = (uint8_t*) malloc(part_size);

    stash_init(stash+pos);

    pos_map[pos] = (int*) malloc(pos_map_size);
    
    File_read_with_map(partition[pos], pos_map[pos], pos);
    
    uint8_t *temp_map = (uint8_t*)pos_map[pos];
    
    otp_crypto(temp_map, key, pos_map_size);
    otp_crypto(partition[pos], key, part_size);
}

void Write_Partition_to_File(uint8_t **partition, Stash *stash, int **pos_map, int pos, uint8_t *key){
    uint8_t *temp_map = (uint8_t*)pos_map[pos];
    otp_crypto(temp_map, key, tree_size * sizeof(int));
    otp_crypto(partition[pos], key, (block_size + 4)*tree_size * Z);
    File_write_with_map(partition[pos], pos_map[pos], pos);
    free(pos_map[pos]);
    free(partition[pos]);
}

void Fully_Shuffle(int *permutation, uint8_t ** public_key){
    // first devide all partitions to sqrt(N) blocks
    
    int sub_block_size = sqrt_int(block_size);
    uint8_t **data_buffer = (uint8_t**)malloc(sub_block_size*sizeof(uint8_t*));
    uint8_t **temp_buffer = (uint8_t**)malloc(sub_block_size*sizeof(uint8_t*));
    
    int data_size = (block_size + 4) * Z * tree_size + tree_size * 4;
    int *new_permutation = (int *)malloc(sub_block_size*sizeof(int));
    
    //allocate buffer for shuffling
    for(int i=0; i<sub_block_size;i++){
        data_buffer[i] = (uint8_t*) malloc(data_size);
        temp_buffer[i] = (uint8_t*) malloc(data_size);
    }
    FILE_INPUT input;
    // phase I, redistribute blocks to different bucket
    for(int i=0; i< sub_block_size; i++){
        // read sqrt of block to memory buffer
        for(int j=0; j<sub_block_size; j++){
            input.address = i*sub_block_size + j;
            input.data_buffer = data_buffer[j];
            input.phase = 1;
            File_read(&input);
            new_permutation[j] = permutation[i*sub_block_size + j];
        }
        Inmemory_shuffle(data_buffer, temp_buffer, new_permutation, public_key);
        //write the shuffled data to memory
        for(int j=0; j<sub_block_size; j++){
            input.address = j*sub_block_size + i;
            input.data_buffer = data_buffer[j];
            input.phase = 2;
            File_write(&input);
            permutation[j*sub_block_size+i] = new_permutation[j];
        }
    }
    
    
    // phase II, reshuffle block inside each bucket
    for(int i=0; i< sub_block_size; i++){
        // read sqrt of block to memory buffer
        for(int j=0; j<sub_block_size; j++){
            input.address = i*sub_block_size + j;
            input.data_buffer = data_buffer[j];
            input.phase = 2;
            File_read(&input);
            new_permutation[j] = permutation[i*sub_block_size + j];
        }
        Inmemory_shuffle(data_buffer, temp_buffer, new_permutation, public_key);
        //write the shuffled data to memory
        for(int j=0; j<sub_block_size; j++){
            input.address = i*sub_block_size + j;
            input.data_buffer = data_buffer[j];
            input.phase = 2;
            File_write(&input);
            permutation[i*sub_block_size + j] = new_permutation[j];
        }
    }
    
    // free data
    for(int i=0; i< sub_block_size; i++){
        free(temp_buffer[i]);
        free(data_buffer[i]);
    }
    free(new_permutation);
    free(temp_buffer);
    free(data_buffer);
}

void Inmemory_shuffle(uint8_t ** data_buffer, uint8_t **temp_buffer, int*new_permutation, uint8_t ** public_key){
    
    // divide the shuffle into log( sqrt(N)  ) step, each step swap sqrt(N)/2 of blocks
    
    int step_num = log_int(sqrt_int(partition_num));
    int group_num = sqrt_int(partition_num);
    int data_size = (block_size + 4) * Z * tree_size + tree_size * 4;
    int pos, pos1, pos2;
    srand(clock());
    int random_key[8];
    int *permutation_buffer = (int*) malloc(sqrt_int(partition_num));
    

    int flag = 0;
    uint8_t *key1, *key2;
    for(int i=0; i<step_num; i++){
        for(int j=0; j<sqrt_int(partition_num)/2; j++){
            pos = (j*2)/group_num * group_num;
            pos1 = ((j*2) - pos)/2;
            pos2 = ((j*2) - pos)/2 + group_num/2;
            // generate random key
            for(int k=0; k<8; k++){
                random_key[k] = rand();
            }
            key1 = (uint8_t*) random_key;
            key2 = (uint8_t*) (random_key + 4);
            // encrypt
            otp_crypto(temp_buffer[2*j + 1], key1, data_size);
            otp_crypto(temp_buffer[2*j], key2, data_size);
            // update key
            for(int k=0; k<16; k++){
                public_key[new_permutation[2*j]][k] ^= key1[k];
                public_key[new_permutation[2*j + 1]][k] ^= key2[k];
            }
            // data shuffle
            if(rand()%2){
                // swap
                if(flag ==0){
                    permutation_buffer[pos1] = new_permutation[2*j + 1];
                    permutation_buffer[pos2] = new_permutation[2*j];
                    memcpy(temp_buffer[pos1], data_buffer[2*j + 1], data_size);
                    memcpy(temp_buffer[pos2], data_buffer[2*j], data_size);
                }else{
                    new_permutation[pos1] = permutation_buffer[2*j +1];
                    new_permutation[pos2] = permutation_buffer[2*j];
                    memcpy(data_buffer[pos1], temp_buffer[2*j + 1], data_size);
                    memcpy(data_buffer[pos2], temp_buffer[2*j], data_size);
                }
            }
            else{
                // swap
                if(flag ==0){
                    permutation_buffer[pos1] = new_permutation[2*j];
                    permutation_buffer[pos2] = new_permutation[2*j + 1];
                    memcpy(temp_buffer[pos1], data_buffer[2*j], data_size);
                    memcpy(temp_buffer[pos2], data_buffer[2*j + 1], data_size);
                }else{
                    new_permutation[pos1] = permutation_buffer[2*j];
                    new_permutation[pos2] = permutation_buffer[2*j + 1];
                    memcpy(data_buffer[pos1], temp_buffer[2*j], data_size);
                    memcpy(data_buffer[pos2], temp_buffer[2*j + 1], data_size);
                }
            }
        }
        group_num = group_num/2;
        flag = !flag;
    }
    free(new_permutation);
}

void Part_Evict(uint8_t **partition, Stash *stash, int **pos_map, int *permutaition, int *in_memory_list, uint8_t ** public_key, uint8_t *aes_key, int phase){
    
    //first re-order the unused partitions
    //second shuffle the in memory partition
    //third evict the in memory data
    //Write data to the correspond partition in file
    int pos;
    int step;
    if(phase == 1){
        pos = sqrt_int(partition_num) - 1;
        step = sqrt_int(partition_num);
    }else{
        pos = partition_num - sqrt_int(partition_num);
        step = 1;
    }
    int data_size = (block_size + 4) * Z * tree_size + tree_size * 4;
    uint8_t ** data_buffer = (uint8_t**) malloc(sqrt_int(partition_num)*sizeof(uint8_t*));
    uint8_t **temp_buffer = (uint8_t**) malloc(sqrt_int(partition_num)*sizeof(uint8_t*));
    int * new_permutation = (int*) malloc(sqrt_int(partition_num) * sizeof(int));
    int flag = 0;
    int swap_buffer;
    char new_file_name[15];
    char old_file_name[15];
    // phase I: divide partition into the end of each sqrt(N) bucket
    // step 1: reorder partitions
    for(int i=0; i< sqrt_int(partition_num); i++){
        if((in_memory_list[i] + 1) %partition_num != 0){
            // not in the target pos, search a empty space to swap
            for(; pos<partition_num && flag == 0; pos += step){
                flag = 1;
                // detect if pos already in the memory
                for(int j=0; j<sqrt_int(partition_num); j++ ){
                    if(in_memory_list[j] == pos)
                        flag=0;
                }
            }
            //swap in_memory_list[i] and pos
            sprintf(old_file_name, "%.txt", pos);
            sprintf(new_file_name, "%.txt", in_memory_list[i]);
            swap_buffer = permutaition[pos];
            permutaition[pos] = permutaition[in_memory_list[i]];
            permutaition[in_memory_list[i]] = swap_buffer;
            in_memory_list[i] = pos;
        }
    }
    // step 2: evict stash, tree, pos_map
    for(int i=0; i<sqrt_int(partition_num); i++){
        data_buffer[i] = malloc(data_size);
        temp_buffer[i] = malloc(data_size);
        tree_free(pos_map[i], partition[i], aes_key, stash + i);
        memcpy(data_buffer[i], pos_map[i], tree_size * 4);
        memcpy(data_buffer[i] + tree_size * 4, partition[i], (block_size + 4) * Z * tree_size);
        otp_crypto(data_buffer[i], public_key[permutaition[in_memory_list[i]]], data_size);
        free(pos_map[i]);
        free(partition[i]);
        new_permutation[i] = permutaition[in_memory_list[i]];
    }
    Inmemory_shuffle(data_buffer, temp_buffer, new_permutation, public_key);
    
    // write partition to file
    if(phase == 1){
        pos = sqrt_int(partition_num) - 1;
    }
    else{
        pos = partition_num - sqrt_int(partition_num);
    }
    FILE_INPUT input;
    for(int i=0; i<sqrt_int(partition_num); i++, pos+=step){
        permutaition[pos] = new_permutation[i];
        input.address = pos;
        input.phase = phase;
        input.data_buffer = data_buffer[i];
        File_write(&input);
    }
    // free data
    for(int i=0; i< sqrt_int(partition_num); i++){
        free(temp_buffer[i]);
        free(data_buffer[i]);
    }
    free(new_permutation);
    free(temp_buffer);
    free(data_buffer);
    free(in_memory_list);
}

void Partition_Access(uint8_t **partition, Stash *stash, int **pos_map, uint8_t *aes_key, int *permutaition, REQ *request, int N){

    for(int i=0; i<N; i++){
        tree_access(request->data[i], partition[i], pos_map[i], &stash[i], request->tree_address[i], request->op[i], aes_key);
    }
    
}
