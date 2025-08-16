
#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <stdint.h>

#include "./chunk.h"

struct image_data {
    int length;
    uint8_t* data;
    uint32_t height;
    uint32_t width;
    uint8_t color_type;
    uint8_t bit_type;
};

struct image_data* init_image_data(int width, int height, int bit_type, int color_type);
void concat_idat_chunks(struct image_data* image_data, struct chunk** idat_chunks, int idat_chunks_length);
int split_data_in_idat_chunks(struct image_data* image_data, struct chunk*** idat_chunks);
int get_image_raw_size(struct image_data* image_data);

#endif  // IMAGE_DATA_H
