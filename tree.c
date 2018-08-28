
#include "tree.h"

extern int partition_num;
extern int tree_size;
extern int block_size;
extern int Z;
extern int user_num;
extern int memory_access_circle;
extern int request_num;


void stash_insert(uint8_t *data, int ID, Stash *stash){
    Stash_Block *new_block = (Stash_Block*) malloc(sizeof(Stash_Block));
    new_block->ID = ID;
    new_block->next = NULL;
    new_block->data = (uint8_t*)malloc(block_size);
    memcpy(new_block->data, data, block_size);
    
    if(stash->size == 0){
        stash->start = new_block;
        stash->end = new_block;
    }
    stash->end->next = new_block;
    stash->end = new_block;
    stash->size += 1;
    
}

void stash_init(Stash *stash){
    stash->start = NULL;
    stash->end = NULL;
    stash->size = 0;
}

Stash_Block* stash_erase(Stash *stash, int pos){
    Stash_Block *temp = stash->start;
    Stash_Block *temp2 = temp;
    if(pos == 0){
        stash->start = stash->start->next;
        temp2 = stash->start;
    }
    else{
        for(int i=0; i<pos; i++){
            temp2 = temp;
            temp = temp->next;
        }
        temp2->next = temp2->next->next;
        temp2 = temp2->next;
    }
    if(temp2->next == NULL){
        stash->end = temp2;
    }
    stash->size = stash->size - 1;
    free(temp->data);
    free(temp);
    return temp2;
}


int get_node(int level, int leaf){
    //                    0
    //          1                      2
    //     3         4         5               6
    // 7      8   9     10  11    12       13      14
    // 0      1   2      3   4     5        6       7
    int max_level = log_int(block_size + 1);
    
    leaf = leaf >> (max_level - level);
    
    return pow_int(level) + leaf - 1;
}


void tree_access(uint8_t *data, uint8_t *tree, int *pos_map, Stash *stash, int address, int isRead, uint8_t *key){
    srand(clock());
    int x = pos_map[address];
    pos_map[address] = rand()%((tree_size + 1)/2);
    
    // fetch bucket from memory
//    for(int i=0; i<tree_size; i++){
//        printf("%d\n", pos_map[i]);
//    }
    uint8_t *temp_bucket = (uint8_t*) malloc((block_size + 4) * Z);
    int level = log_int(tree_size + 1);
    int pos;
    int temp_x = x;
    int *ID;
    int aa;

    for(int i=level; i > 0; i--){
        pos = pow_int(i - 1) + 1 + temp_x;
        temp_x = temp_x / 2;
        memcpy(temp_bucket, tree + i * (block_size + 4) * Z, (block_size + 4) * Z);
        
        array_decrypt(temp_bucket, key, (block_size + 4) * Z);
        
        for(int j=0; j<Z; j++){
            ID = (int*)(temp_bucket + j * (block_size + 4));
            aa = *ID;
//            for(int k=0; k<block_size; k++){
//                printf("%d ",temp_bucket[ j * (block_size + 4) + 4+ k]);
//            }
//            printf("\n\n");
            if(*ID != -1){
                stash_insert(temp_bucket + j * (block_size + 4) + 4, *ID, stash);
            }
        }
    }

    free(temp_bucket);
    
    
    // write or read data in stash
    pos = -1;
    Stash_Block * temp_block = stash->start;
    for(int i=0; i< stash->size; i++){
        if(temp_block->ID == address){
            pos = i;
            break;
        }
        temp_block = temp_block->next;
    }
    
    if(isRead == 1){
        // read data from stash;
        // if not exitst, create one
        if(pos == -1){
            temp_bucket = (uint8_t*) malloc(block_size);
            for(int i=0; i<block_size; i++)
                temp_bucket[i] = 0;
            memcpy(data, temp_bucket, block_size);
            stash_insert(temp_bucket, address, stash);
            free(temp_bucket);
        }
        else{
            memcpy(data, temp_block->data, block_size);
        }
    }
    else{
        // write data to stash
        if(pos == -1){
            stash_insert(data, address, stash);
        }
        else{
            memcpy(temp_block->data, data, block_size);
        }
    }
    // create a blank block
    uint8_t *blank_block = (uint8_t*) malloc(block_size);
    for(int i=0; i<block_size; i++){
        blank_block[i] = 0;
    }
    // write stash to path
    temp_bucket = (uint8_t*) malloc((block_size + 4) * Z);
    int num;
    int buffer;
    for(int i=level; i>0; i--){
        temp_block = stash->start;
        num = 0;
        pos = get_node(i, x);
        
        for(int j=0 ; j<stash->size; j++){
            if(num < Z && get_node(i, pos_map[temp_block->ID]) == pos){
                buffer = temp_block->ID;
                memcpy(temp_bucket + num*(block_size + 4), &buffer, 4);
                memcpy(temp_bucket + num*(block_size + 4) + 4, temp_block->data, block_size);
                num++;
                temp_block = stash_erase(stash, j);
            }
            else {
                temp_block = temp_block->next;
            }
        }
        for(; num<Z; num++){
            buffer = -1;
            memcpy(temp_bucket + num*(block_size + 4), &buffer, 4);
            memcpy(temp_bucket + num*(block_size + 4) + 4, blank_block, block_size);
        }
        
        array_encrypt(temp_bucket, key, (block_size + 4) * Z);
        memcpy(tree + pos * (block_size + 4) * Z, temp_bucket, (block_size + 4) * Z);
    }
}

void tree_init(int *pos_map, uint8_t *part, uint8_t *key){
    
    int temp, level;
    rand_permutation(pos_map, tree_size);
    
    
    int bucket_size = (block_size+4)*Z;
    int ID;
    uint8_t *encry_data = malloc((block_size+4)*Z);
    
    for(int i=0; i<tree_size; i++){
        for(int j=0; j<Z; j++){
            ID = -1;
            memcpy(encry_data + j*(block_size+4), &ID, 4);
            rand_array(encry_data +  j*(block_size+4) + 4, block_size);
        }
        array_encrypt(encry_data, key, bucket_size);
        memcpy(part + i*bucket_size, encry_data, bucket_size);
    }
    //                    0
    //          1                      2
    //          0                      1
    //     3         4         5               6
    //     0         1         2               3
    // 7      8   9     10  11    12       13      14
    // 0      1   2      3   4     5        6       7
    for(int j=0; j<tree_size; j++){
        temp = pos_map[j];
        level = log_int(temp);
        temp = temp - pow_int(level) + 1;
        for(int k=level; k< log_int(tree_size) - 1; k++){
            temp = temp*2 + rand()%2;
        }
        pos_map[j] = temp;
    }
    free(encry_data);
}

void tree_free(int *pos_map, uint8_t *tree, uint8_t *key, Stash *stash){
    srand(clock());
    Stash_Block *temp = stash->start;
    int level;
    int max_level = log_int(block_size+ 1);
    int flag = 0;
    int pos;
    int *ID;
    uint8_t *temp_bucket = (uint8_t*) malloc((block_size+4)*Z);
    
    
    for(int i=0; i<stash->size; i++){
        flag = 0;
        while(flag == 0){
            level = rand()%max_level;
            pos = get_node(level, pos_map[temp->ID]);
            memcpy(temp_bucket, tree + pos * (block_size+4)*Z, (block_size+4)*Z);
            array_decrypt(temp_bucket, key, (block_size+4)*Z);
            for(int j=0; j<Z; j++){
                ID = (int*)(temp_bucket + j*(block_size+4));
                if(*ID == -1){
                    flag = 1;
                    memcpy(temp_bucket + j*(block_size+4) + 4, temp->data, (block_size+4));
                    *ID = temp->ID;
                    array_encrypt(temp_bucket, key, (block_size+4)*Z);
                    memcpy(tree + pos * (block_size+4)*Z, temp_bucket, (block_size+4)*Z);
                    break;
                }
            }
        }
        temp = stash_erase(stash, i);
        
    }
}
