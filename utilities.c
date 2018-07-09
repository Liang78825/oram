

#include "utilities.h"

void array_encrypt(uint8_t *data, uint8_t *key, int size){
    
//    uint8_t *computed_cipher = (uint8_t*) malloc(size);
//    __m128i key_schedule[20];
//    aes128_load_key(key,key_schedule);
//    for(int i=0; i< size/16; i++){
//        aes128_enc(key_schedule,data + i*16,computed_cipher + i*16);
//    }
//    memcpy(data, computed_cipher, size);
//    free(computed_cipher);
}


void array_decrypt(uint8_t *data, uint8_t *key, int size){
    
//    uint8_t *computed_plain = (uint8_t*) malloc(size);
//    __m128i key_schedule[20];
//    aes128_load_key(key,key_schedule);
//    for(int i=0; i< size/16; i++){
//        aes128_dec(key_schedule,data + i*16,computed_plain + i*16);
//    }
//    memcpy(data, computed_plain, size);
//    free(computed_plain);
}

void otp_crypto(uint8_t *data, uint8_t *key, int size){
    int *key_int = (int*) key;
    int *data_int = (int*) data;
    for(int i=0; i<size/16; i = i+4){
        data_int[i*4] = data_int[i*4] ^ key_int[0];
        data_int[i*4 + 1] = data_int[i*4 + 1] ^ key_int[1];
        data_int[i*4 + 2] = data_int[i*4 + 2] ^ key_int[2];
        data_int[i*4 + 3] = data_int[i*4 + 3] ^ key_int[3];
    }
}


void rand_permutation(int *data, int size){
    srand(clock());
    int *seq = (int*) malloc(size*sizeof(int));
    for(int i=0; i<size; i++){
        seq[i] = i;
    }
    int temp;
    for(int i=0; i<size; i++){
        temp = rand()%(size-i);
        data[i] = seq[temp];
        seq[temp] = seq[size-i-1];
        seq[size-i-1] = data[i];
    }
    free(seq);
}

void rand_array(uint8_t *data, int size){
    srand(clock());
    int *temp = (int*)data;
    for(int i=0; i<size/4; i++){
        temp[i] = rand();
    }
}

void key_generate(unsigned char **key, int num){
    srand(clock());
    for(int i=0; i<num; i++){
        
        key[i] = (unsigned char*)malloc(32* sizeof(unsigned char));
        
        for(int j=0; j<32; j++){
            key[i][j] = rand()%256;
        }
    }
}

int sqrt_int(int n) {
    for(int i=0; i<=n; i++){
        if(i*i>=n)
            return i;
    }
    return 0;
}

int pow_int(int n){
    int result = 1;
    for(int i=n; i>0; i--){
        result = result*2;
    }
    return result;
}

int log_int(int n){
    int log_value = -1;
    while(n){
        log_value++;
        n >>= 1;
    }
    return log_value;
}
