#include "../include/chunk.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "../include/error.h"
#include "../include/utils.h"

const char IHDR_CHUNK_ID[4] = {'I', 'H', 'D', 'R'};
const char IDAT_CHUNK_ID[4] = {'I', 'D', 'A', 'T'};
const char IEND_CHUNK_ID[4] = {'I', 'E', 'N', 'D'};

void print_chunk_data(struct chunk* chunk) {
    printf("Chunk ID: %s\n", chunk->id);
    printf("Chunk Length: %u\n", chunk->length);
    printf("Chunk CRC: %u\n", chunk->crc);
    printf("Chunk Data: ");
    for (int i = 0; i < (int)(chunk->length); i++) {
        printf("%02x ", chunk->data[i]);
    }
    printf("\n");
}

uint32_t create_chunk_crc(const char* id, const uint8_t* data, int length) {
    uint32_t computed_crc32 = crc32(0L, Z_NULL, 0);
    computed_crc32 = crc32(computed_crc32, (const Bytef*)id, CHUNK_ID_SIZE);
    computed_crc32 = crc32(computed_crc32, data, length);
    return computed_crc32;
}

int check_chunk_crc(struct chunk* chunk) {
    if (create_chunk_crc(chunk->id, chunk->data, chunk->length) != chunk->crc) {
        return -1;
    }
    return 0;
}

int are_chunk_id_equal(const char* id, const char* id2) {
    for (int i = 0; i < CHUNK_ID_SIZE; i++) {
        if (id[i] != id2[i]) {
            return 0;
        }
    }
    return 1;
}

struct chunk* init_chunk(const char* id, const uint8_t* data, int data_length, uint32_t crc) {
    struct chunk* new_chunk = malloc(sizeof(struct chunk));
    if (!new_chunk)
        exit_memory_allocation_error();
    new_chunk->id = malloc(sizeof(char) * 4);
    if (!new_chunk->id) {
        free(new_chunk);
        exit_memory_allocation_error();
    }
    memcpy(new_chunk->id, id, 4);
    new_chunk->length = data_length;
    new_chunk->data = malloc(data_length);
    if (!new_chunk->data) {
        free(new_chunk->id);
        free(new_chunk);
        exit_memory_allocation_error();
    }
    memcpy(new_chunk->data, data, data_length);
    new_chunk->crc = crc;
    return new_chunk;
}

struct chunk* init_idat_chunk(const uint8_t* data, int data_length) {
    return init_chunk(IDAT_CHUNK_ID, data, data_length, create_chunk_crc(IDAT_CHUNK_ID, data, data_length));
}

void free_chunk(struct chunk* chunk) {
    if (!chunk)
        return;
    if (chunk->id)
        free(chunk->id);
    if (chunk->data)
        free(chunk->data);
    free(chunk);
}

uint32_t read_endian_number(int fd, int sz) {
    uint32_t length_unconverted;
    int read_size = read(fd, &length_unconverted, sz);
    if (read_size != sz)
        exit_error_while_reading_file();
    uint32_t length_converted =
        convert_endian_to_uint32_number(length_unconverted);
    return length_converted;
}

uint32_t get_chunk_length(int fd) {
    return read_endian_number(fd, CHUNK_LENGTH_SIZE);
}

char* get_chunk_id(int fd) {
    char* id = malloc(sizeof(char) * 4);
    int sz = read(fd, id, CHUNK_ID_SIZE);
    if (sz != 4)
        exit_file_read_error("chunk id");
    return id;
}

uint8_t* get_chunk_data(int fd, uint32_t data_length) {
    uint8_t* buff = malloc(sizeof(uint8_t) * data_length);
    int sz = read(fd, buff, data_length);
    if ((uint32_t)sz != data_length)
        exit_file_read_error("chunk data");
    return buff;
}

uint32_t get_chunk_crc(int fd) {
    return read_endian_number(fd, CHUNK_CRC_SIZE);
}

void read_chunk(int fd, struct chunk** chunk) {
    uint32_t length = get_chunk_length(fd);
    char* id = get_chunk_id(fd);
    uint8_t* data = get_chunk_data(fd, length);
    uint32_t crc = get_chunk_crc(fd);

    (*chunk) = init_chunk(id, data, length, crc);
    if (check_chunk_crc(*chunk) < 0)
        exit_error_while_reading_file();
    printf("length: %u\n", (*chunk)->length);
    printf("id: %s\n", (*chunk)->id);
    printf("crc: ok\n");
}

void write_uint32_number(int fd, uint32_t n, int sz) {
    uint32_t converted_n = convert_uint32_to_endian_number(n);
    int write_sz = write(fd, &converted_n, sz);
    if (write_sz != sz)
        exit_error_while_reading_file();
}

void write_chunk_length(int fd, uint32_t length) {
    write_uint32_number(fd, length, CHUNK_LENGTH_SIZE);
}

void write_chunk_id(int fd, const char* id) {
    int sz = write(fd, id, CHUNK_ID_SIZE);
    if (sz != CHUNK_ID_SIZE)
        exit_error_while_reading_file();
}

void write_chunk_data(int fd, uint8_t* data, int data_length) {
    int sz = write(fd, data, data_length);
    if (sz != data_length)
        exit_error_while_reading_file();
}

void write_chunk_crc(int fd, uint32_t crc) {
    write_uint32_number(fd, crc, CHUNK_CRC_SIZE);
}

void write_chunk(int fd, struct chunk* chunk) {
    if (are_chunk_id_equal(chunk->id, IHDR_CHUNK_ID)) {
        print_chunk_data(chunk);
    }
    write_chunk_length(fd, chunk->length);
    write_chunk_id(fd, chunk->id);
    write_chunk_data(fd, chunk->data, chunk->length);
    write_chunk_crc(fd, chunk->crc);
}
