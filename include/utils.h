
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define DEFAULT_ENDIAN_NUMBER_SIZE 4

uint32_t convert_uint32_to_endian_number(uint32_t n);
uint32_t convert_endian_to_uint32_number(uint32_t n);

int decompress_data(const uint8_t* data_in, int data_in_length, uint8_t* data_out, int data_out_length);
int compress_data(const uint8_t* data_in, int data_in_length, uint8_t** data_out);

#endif // UTILS_H
