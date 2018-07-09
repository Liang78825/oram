

#ifndef utilities_h
#define utilities_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "aesni.h"

#endif /* utilities_h */

//return 0 if no error
//1 if encryption failed
//2 if decryption failed
//3 if both failed
void array_encrypt(uint8_t *data, uint8_t *key, int size);


void array_decrypt(uint8_t *data, uint8_t *key, int size);

void otp_crypto(uint8_t *data, uint8_t *key, int size);


void rand_permutation(int *data, int size);

void rand_array(uint8_t *data, int size);

int log_int(int n);

int sqrt_int(int n);

int pow_int(int n);

void key_generate(unsigned char **key, int num);


