
#include "file.h"

extern int partition_num;
extern int tree_size;
extern int block_size;
extern int Z;
extern int user_num;
extern int memory_access_circle;
extern int request_num;

void FileInit(unsigned char **public_key, uint8_t* aes_key){
    int *pos_map = (int*)malloc(tree_size*sizeof(int));
    uint8_t *part = (uint8_t*)malloc(((block_size + 4)*Z)*tree_size);
    uint8_t *encry_pos_map = (uint8_t *)pos_map;
    for(int i=0; i<partition_num; i++){
        tree_init(pos_map, part, aes_key);
        otp_crypto(encry_pos_map, public_key[i], tree_size*sizeof(int));
        otp_crypto(part, public_key[i], ((block_size + 4)*Z)*tree_size);
        File_write_with_map(part, pos_map, i);
    }
    free(pos_map);
    free(part);
}

void File_read_with_map(uint8_t *data, int *pos_map, int address){
    
    int map_size = tree_size * 4;
    int part_size = (block_size + 4)*Z * tree_size;
    
    FILE *fd = NULL;
    char file_name[10] = "";
    sprintf(file_name, "1-%d.txt", address);
    fd = fopen(file_name, "r");
    fread(pos_map, 1, map_size, fd);
    fread(data, 1, part_size, fd);
    fclose(fd);
   // remove(file_name);
}
void File_write_with_map(uint8_t *data, int *pos_map, int address){
    
    int map_size = tree_size * 4;
    int part_size = (block_size + 4)*Z * tree_size;
    
    FILE *fd = NULL;
    char file_name[10] = "";
    sprintf(file_name, "1-%d.txt", address);
    fd = fopen(file_name, "w");
    fwrite(pos_map, 1, map_size, fd);
    fwrite(data, 1, part_size, fd);
    fclose(fd);
}

void *File_read(void *input_t){
    FILE_INPUT *input = (FILE_INPUT*) input_t;
    int data_size = tree_size * 4 + (block_size + 4)*Z * tree_size;
    FILE *fd = NULL;
    char file_name[15] = "";
    sprintf(file_name, "%d-%d.txt", input->phase, input->address);
    fd = fopen(file_name, "r");
    fread(input->data_buffer, 1, data_size, fd);
    fclose(fd);
//    remove(file_name);
    return NULL;
}

void *File_write(void *input_t){
    FILE_INPUT *input = (FILE_INPUT*) input_t;
    int data_size = tree_size * 4 + (block_size + 4)*Z * tree_size;
    FILE *fd = NULL;
    char file_name[15] = "";
    sprintf(file_name, "%d-%d.txt", input->phase, input->address);
    fd = fopen(file_name, "w");
    fwrite(input->data_buffer, 1, data_size, fd);
    fclose(fd);
    return NULL;
}
