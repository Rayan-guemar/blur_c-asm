#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./chunk.h"
#include "./error.h"
#include "./image_data.h"

struct image_data* init_image_data(int width, int height, int bit_type, int color_type) {
    struct image_data* img = malloc(sizeof(struct image_data));
    if (img == NULL)
        exit_memory_allocation_error();
    img->width = width;
    img->height = height;
    img->bit_type = bit_type;
    img->color_type = color_type;
    img->data = NULL;
    img->length = 0;
    return img;
}

void free_image_data(struct image_data* image_data) {
    if (!image_data) {
        return;
    }

    if (image_data->data) {
        free(image_data->data);
    }

    free(image_data);
}

void concat_idat_chunks(struct image_data* image_data, struct chunk** idat_chunks, int idat_chunks_length) {
    int length = 0;

    for (int i = 0; i < idat_chunks_length; i++) {
        length += idat_chunks[i]->length;
    }

    uint8_t* data = malloc(length * sizeof(uint8_t));

    if (!data) {
        exit_memory_allocation_error();
    }

    int current_length = 0;

    for (int i = 0; i < idat_chunks_length; i++) {
        memcpy(data + current_length, idat_chunks[i]->data, idat_chunks[i]->length);
        current_length += idat_chunks[i]->length;
    }

    image_data->data = data;
    image_data->length = length;
}

int split_data_in_idat_chunks(struct image_data* image_data, struct chunk*** idat_chunks) {
    int idat_chunks_length = (int)ceil((double)image_data->length / CHUNK_IDAT_MAX_LENGTH);

    *idat_chunks = malloc(sizeof(struct chunk*) * idat_chunks_length);

    if (*idat_chunks == NULL) {
        exit_memory_allocation_error();
    }

    for (int i = 0; i < (idat_chunks_length); i++) {
        int remaining = image_data->length - i * CHUNK_IDAT_MAX_LENGTH;

        if (remaining <= 0) {
            break;
        }

        uint8_t* buff = malloc(remaining);

        if (buff == NULL) {
            exit_memory_allocation_error();
        }

        memcpy(buff, image_data->data + i * CHUNK_IDAT_MAX_LENGTH, remaining);

        struct chunk* test = init_idat_chunk(buff, remaining);
        (*idat_chunks)[i] = test;

        free(buff);
    }

    return idat_chunks_length;
}

int get_color_type_byte(int color_type) {
    switch (color_type) {
        case 0:
            return 1;
            break;
        case 2:
            return 3;
            break;
        case 3:
            return 1;
            break;
        case 4:
            return 2;
            break;
        case 6:
            return 4;
            break;

        default:
            exit_error_while_reading_file();
            return -1;
            break;
    }
}

int get_image_raw_size(struct image_data* image_data) {
    int heigth = image_data->height;
    int width = image_data->width;
    int byte_number = image_data->bit_type / 8;
    int color_type_byte = get_color_type_byte(image_data->color_type);

    return heigth * (width * byte_number * color_type_byte + 1);
}
