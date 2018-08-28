

#ifndef file_h
#define file_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utilities.h"
#include "aesni.h"
#include "tree.h"



typedef struct file_input{
    uint8_t* data_buffer;
    int address;
    int phase;
} FILE_INPUT;

#endif /* file_h */

void FileInit(unsigned char **public_key, uint8_t* aes_key);

void File_read_with_map(uint8_t *data, int *pos_map, int address);

void File_write_with_map(uint8_t *data, int *pos_map, int address);

void *File_read(void *input);

void *File_write(void *input);
