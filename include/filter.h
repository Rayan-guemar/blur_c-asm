
#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

#include "image_data.h"

#define PIXEL_SIZE 3

#define FILTER_NONE 0
#define FILTER_SUB 1
#define FILTER_UP 2
#define FILTER_AVERAGE 3
#define FILTER_PAETH 4

void defilter_data(struct image_data* img);
void refilter_data(const struct image_data* img, uint8_t* buff);

#endif  // FILTER_H
