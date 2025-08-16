#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/chunk.h"
#include "include/error.h"
#include "include/filter.h"
#include "include/image_data.h"
#include "include/utils.h"

const uint8_t PNG_SIG[PNG_SIG_LENGTH] = {137, 80, 78, 71, 13, 10, 26, 10};

void read_png_header(int fd) {
    uint8_t sig[PNG_SIG_LENGTH];
    int sz = read(fd, sig, PNG_SIG_LENGTH);

    if (sz != PNG_SIG_LENGTH) {
        exit_file_read_error("image.png");
    }

    for (int i = 0; i < sz; i++) {
        if (sig[i] != PNG_SIG[i]) {
            exit_wrong_file_type();
        }
    }
}

void read_chunks(int fd, struct chunk** ihdr_chunk, struct chunk*** idat_chunks, int* idat_chunks_length, struct chunk*** optionnal_chunks, int* optionnal_chunks_length, struct chunk** iend_chunk) {
    read_chunk(fd, ihdr_chunk);

    if (!are_chunk_id_equal((*ihdr_chunk)->id, (char*)IHDR_CHUNK_ID)) {
        exit_error_while_reading_file();
    }

    *idat_chunks = NULL;
    *idat_chunks_length = 0;
    *optionnal_chunks = NULL;
    *optionnal_chunks_length = 0;
    *iend_chunk = NULL;

    while (1) {
        struct chunk* current_chunk = NULL;  // Placeholder, will be filled by read_chunk
        read_chunk(fd, &current_chunk);

        if (are_chunk_id_equal(current_chunk->id, IDAT_CHUNK_ID)) {
            (*idat_chunks_length)++;
            *idat_chunks = realloc(*idat_chunks, sizeof(struct chunk*) * (*idat_chunks_length));
            (*idat_chunks)[(*idat_chunks_length) - 1] = current_chunk;
        } else if (are_chunk_id_equal(current_chunk->id, IEND_CHUNK_ID)) {
            *iend_chunk = current_chunk;
            break;
        } else {
            (*optionnal_chunks_length)++;
            *optionnal_chunks = realloc(*optionnal_chunks, sizeof(struct chunk*) * (*optionnal_chunks_length));
            (*optionnal_chunks)[(*optionnal_chunks_length) - 1] = current_chunk;
        }
    }
}

void extract_image_info(struct chunk* ihdr_chunk, uint32_t* width, uint32_t* height, uint8_t* bit_type, uint8_t* color_type) {
    uint32_t unconverted_width, unconverted_height;
    memcpy(&unconverted_width, ihdr_chunk->data, DEFAULT_ENDIAN_NUMBER_SIZE);
    memcpy(&unconverted_height, ihdr_chunk->data + 4, DEFAULT_ENDIAN_NUMBER_SIZE);

    *width = convert_endian_to_uint32_number(unconverted_width);
    *height = convert_endian_to_uint32_number(unconverted_height);

    *bit_type = ihdr_chunk->data[2 * DEFAULT_ENDIAN_NUMBER_SIZE];
    *color_type = ihdr_chunk->data[2 * DEFAULT_ENDIAN_NUMBER_SIZE + 1];
}

void write_png_file(const char* filename, struct chunk* ihdr_chunk, struct chunk** optionnal_chunks, int optionnal_chunks_length, struct chunk** idat_chunks, int idat_chunks_length, struct chunk* iend_chunk) {
    int fd_copy = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (fd_copy == -1) {
        exit_file_open_error(filename);
    }

    int sz = write(fd_copy, PNG_SIG, PNG_SIG_LENGTH);

    if (sz != PNG_SIG_LENGTH) {
        exit_file_write_error(filename);
    }

    write_chunk(fd_copy, ihdr_chunk);

    for (int i = 0; i < optionnal_chunks_length; i++) {
        write_chunk(fd_copy, optionnal_chunks[i]);
    }

    for (int i = 0; i < idat_chunks_length; i++) {
        write_chunk(fd_copy, idat_chunks[i]);
    }

    write_chunk(fd_copy, iend_chunk);

    close(fd_copy);
}

void free_chunks(struct chunk* ihdr_chunk, struct chunk** idat_chunks, int idat_chunks_length, struct chunk** optionnal_chunks, int optionnal_chunks_length, struct chunk* iend_chunk) {
    if (ihdr_chunk) {
        free_chunk(ihdr_chunk);
    }

    if (idat_chunks) {
        for (int i = 0; i < idat_chunks_length; i++) {
            if (idat_chunks[i])
                free_chunk(idat_chunks[i]);
        }
        free(idat_chunks);
    }

    if (optionnal_chunks) {
        for (int i = 0; i < optionnal_chunks_length; i++) {
            if (optionnal_chunks[i])
                free_chunk(optionnal_chunks[i]);
        }
        free(optionnal_chunks);
    }

    if (iend_chunk) {
        free_chunk(iend_chunk);
    }
}

int main() {
    char* filename = "image.png";
    int fd = open(filename, O_RDONLY);

    if (fd == -1) {
        exit_file_open_error(filename);
    }

    read_png_header(fd);

    struct chunk* ihdr_chunk;
    struct chunk **idat_chunks, **optionnal_chunks, *iend_chunk;
    int idat_chunks_length, optionnal_chunks_length;

    read_chunks(fd, &ihdr_chunk, &idat_chunks, &idat_chunks_length, &optionnal_chunks, &optionnal_chunks_length, &iend_chunk);

    uint32_t width, height;
    uint8_t bit_type, color_type;
    extract_image_info(ihdr_chunk, &width, &height, &bit_type, &color_type);

    if (color_type != 2) {
        exit_error_while_reading_file();
    }

    struct image_data* image_data = init_image_data(width, height, bit_type, color_type);

    concat_idat_chunks(image_data, idat_chunks, idat_chunks_length);

    int image_raw_size = get_image_raw_size(image_data);
    uint8_t* decompressed_data_buffer = malloc(image_raw_size);
    if (!decompressed_data_buffer) {
        exit_memory_allocation_error();
    }
    int buffer_length = decompress_data(image_data->data, image_data->length, decompressed_data_buffer, image_raw_size);

    printf("Decompressed data length: %d\n", buffer_length);
    if (buffer_length < 0) {
        free(decompressed_data_buffer);
        exit_memory_allocation_error();
    }
    image_data->length = buffer_length;
    image_data->data = decompressed_data_buffer;

    defilter_data(image_data);

    uint8_t* refiltered_data_buffer = malloc(image_data->length * sizeof(uint8_t));
    refilter_data(image_data, refiltered_data_buffer);

    image_data->data = refiltered_data_buffer;

    uint8_t* compressed_data_buffer;
    buffer_length = compress_data(image_data->data, image_data->length, &compressed_data_buffer);

    printf("Compressed data length: %d\n", buffer_length);
    if (buffer_length < 0) {
        free(decompressed_data_buffer);
        exit_memory_allocation_error();
    }

    image_data->length = buffer_length;

    free(decompressed_data_buffer);

    struct chunk** new_idat_chunks;

    image_data->data = compressed_data_buffer;
    int new_idat_chunks_length = split_data_in_idat_chunks(image_data, &new_idat_chunks);

    write_png_file("copy.png", ihdr_chunk, optionnal_chunks, optionnal_chunks_length, new_idat_chunks, new_idat_chunks_length, iend_chunk);

    if (new_idat_chunks) {
        printf("freeing new IDAT chunks\n");
        for (int i = 0; i < new_idat_chunks_length; i++) {
            printf("freeing chunk %d\n", i);
            if (new_idat_chunks[i])
                free_chunk(new_idat_chunks[i]);
        }
        printf("freeing new IDAT chunks array\n");
        free(new_idat_chunks);
    }

    printf("freeing image data\n");
    free(image_data);
    free(compressed_data_buffer);
    close(fd);

    return 0;
}
