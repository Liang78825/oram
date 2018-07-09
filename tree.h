

#ifndef tree_h
#define tree_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utilities.h"
#include "aesni.h"
#endif /* tree_h */

typedef struct stash_block{
    int ID;
    struct stash_block *next;
    uint8_t *data;
} Stash_Block;

typedef struct stash_p{
    Stash_Block *start;
    Stash_Block *end;
    int size;
} Stash;



void stash_insert(uint8_t *data, int ID, Stash *stash);

void stash_init(Stash *stash);

Stash_Block* stash_erase(Stash *stash, int pos);

int get_node(int level, int leaf);

void tree_access(uint8_t *data, uint8_t *tree, int *pos_map, Stash *stash, int address, int isRead, uint8_t *key);

void tree_init(int *pos_map, uint8_t *part, uint8_t *key);

void tree_free(int *pos_map, uint8_t *tree, uint8_t *key, Stash *stash);
