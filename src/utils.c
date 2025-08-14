#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include "../include/error.h"
#include "../include/utils.h"

uint32_t convert_uint32_to_endian_number(uint32_t n) {
    return htonl(n);
}

uint32_t convert_endian_to_uint32_number(uint32_t n) {
    return ntohl(n);
}

int decompress_data(const uint8_t* data_in, int data_in_length, uint8_t* data_out, int data_out_length) {
    z_stream stream = {0};

    if (data_in == NULL || data_out == NULL) {
        exit_memory_allocation_error();
    }

    stream.next_in = (Bytef*)data_in;
    stream.avail_in = data_in_length;

    stream.next_out = (Bytef*)data_out;
    stream.avail_out = data_out_length;

    if (inflateInit(&stream) != Z_OK) {
        exit_error_while_reading_file();
    }

    if (inflate(&stream, Z_FINISH) != Z_STREAM_END) {
        inflateEnd(&stream);
        exit_error_while_reading_file();
    }

    int decompressed_data_size = stream.total_out;
    inflateEnd(&stream);

    return decompressed_data_size;
}

int compress_data(const uint8_t* data_in, int data_in_length, uint8_t** data_out) {
    z_stream stream = {0};

    if (deflateInit(&stream, Z_BEST_COMPRESSION) != Z_OK) {
        exit_error_while_reading_file();
    }
    uLong bound = compressBound(data_in_length);
    *data_out = (Bytef*)malloc(bound);
    if (*data_out == NULL) {
        deflateEnd(&stream);
        exit_memory_allocation_error();
    }

    if (data_in == NULL) {
        deflateEnd(&stream);
        free(*data_out);
        exit_memory_allocation_error();
    }

    stream.next_in = (Bytef*)data_in;
    stream.avail_in = data_in_length;

    stream.next_out = *data_out;
    stream.avail_out = bound;
    stream.avail_in = data_in_length;

    stream.next_out = *data_out;
    stream.avail_out = bound;

    int ret = deflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&stream);
        exit_error_while_reading_file();
    }

    int compressed_data_size = stream.total_out;
    deflateEnd(&stream);

    return compressed_data_size;
}