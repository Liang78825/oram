
#ifndef partition_h
#define partition_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include "utilities.h"
#include "aesni.h"
#include "scheduler.h"
#include "file.h"

#endif /* partition_h */

void Write_Partition(uint8_t **partition, Stash *stash, int **pos_map, uint8_t *key, uint8_t *input, int pos);

void Load_Partition(uint8_t **partition, Stash *stash, int **pos_map, uint8_t *key, uint8_t *input, int pos);

void PartInit(uint8_t **partition, Stash *stash, int **pos_map);

void Fully_Shuffle(int *permutation, uint8_t ** public_key);

void Inmemory_shuffle(uint8_t ** data_buffer, uint8_t **temp_buffer, int*new_permutation, uint8_t ** public_key);

void Part_Evict(uint8_t **partition, Stash *stash, int **pos_map, int *permutaition, int *in_memory_list, uint8_t ** public_key, uint8_t *aes_key, int phase);

void Partition_Access(uint8_t **partition, Stash *stash, int **pos_map, uint8_t *aes_key, int *permutaition, REQ request, int N);

