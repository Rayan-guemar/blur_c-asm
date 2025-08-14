#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

extern const uint8_t PNG_SIG[8];
#define PNG_SIG_LENGTH 8

#define CHUNK_LENGTH_SIZE 4
#define CHUNK_ID_SIZE 4
#define CHUNK_CRC_SIZE 4

#define CHUNK_IDAT_MAX_LENGTH 16384

extern const char IHDR_CHUNK_ID[4];
extern const char IDAT_CHUNK_ID[4];
extern const char IEND_CHUNK_ID[4];

struct chunk {
    uint32_t length;
    char* id;
    uint8_t* data;
    uint32_t crc;
};

struct chunk* init_chunk(const char* id, const uint8_t* data, int data_length, uint32_t crc);
struct chunk* init_idat_chunk(const uint8_t* data, int data_length);
void free_chunk(struct chunk* chunk);

void read_chunk(int fd, struct chunk** chunk);
void write_chunk(int fd, struct chunk* chunk);
int are_chunk_id_equal(const char* id, const char* id2);

#endif  // CHUNK_H
