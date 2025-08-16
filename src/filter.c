#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/error.h"
#include "../include/filter.h"
#include "../include/image_data.h"

int get_pixel_pos(const struct image_data* img, int x, int y) {
    if (x < 0 || x >= (int)img->width || y < 0 || y >= (int)img->height) {
        return -1;
    }
    return y * (img->width * PIXEL_SIZE + 1) + 1 + x * PIXEL_SIZE;
}

int get_left_pixel_pos(const struct image_data* img, int x, int y) {
    if (x < 1 || x >= (int)img->width || y < 0 || y >= (int)img->height) {
        return -1;
    }
    return get_pixel_pos(img, x - 1, y);
}

int get_top_pixel_pos(const struct image_data* img, int x, int y) {
    if (x < 0 || x >= (int)img->width || y < 1 || y >= (int)img->height) {
        return -1;
    }
    return get_pixel_pos(img, x, y - 1);
}

int get_top_left_pixel_pos(const struct image_data* img, int x, int y) {
    if (x < 1 || x >= (int)img->width || y < 1 || y >= (int)img->height) {
        return -1;
    }
    return get_pixel_pos(img, x - 1, y - 1);
}

void defilter_sub(struct image_data* img, int x, int y) {
    if (x == 0) {
        return;
    }

    int pos = get_pixel_pos(img, x, y);
    int left_pos = get_left_pixel_pos(img, x, y);

    if (pos < 0 || left_pos < 0)
        return;

    uint8_t* cur = img->data + pos;
    uint8_t* left = img->data + left_pos;

    for (int i = 0; i < PIXEL_SIZE; i++) {
        cur[i] += left[i];
    }
}

void defilter_up(struct image_data* img, int x, int y) {
    if (y == 0) {
        return;
    }

    int pos = get_pixel_pos(img, x, y);
    int top_pos = get_top_pixel_pos(img, x, y);

    if (pos < 0 || top_pos < 0)
        return;

    uint8_t* cur = img->data + pos;
    uint8_t* top = img->data + top_pos;

    for (int i = 0; i < PIXEL_SIZE; i++) {
        cur[i] += top[i];
    }
}

void defilter_average(struct image_data* img, int x, int y) {
    int pos = get_pixel_pos(img, x, y);
    int left_pos = get_left_pixel_pos(img, x, y);
    int top_pos = get_top_pixel_pos(img, x, y);

    if (pos < 0)
        return;

    uint8_t* cur = img->data + pos;

    for (int i = 0; i < PIXEL_SIZE; i++) {
        uint8_t left = (left_pos >= 0) ? img->data[left_pos + i] : 0;
        uint8_t top = (top_pos >= 0) ? img->data[top_pos + i] : 0;
        cur[i] += (left + top) / 2;
    }
}

void defilter_paeth(struct image_data* img, int x, int y) {
    int pos = get_pixel_pos(img, x, y);
    int left_pos = get_left_pixel_pos(img, x, y);
    int top_pos = get_top_pixel_pos(img, x, y);
    int top_left_pos = get_top_left_pixel_pos(img, x, y);

    if (pos < 0)
        return;

    uint8_t* cur = img->data + pos;

    for (int i = 0; i < PIXEL_SIZE; i++) {
        int left = (left_pos >= 0) ? img->data[left_pos + i] : 0;
        int top = (top_pos >= 0) ? img->data[top_pos + i] : 0;
        int top_left = (top_left_pos >= 0) ? img->data[top_left_pos + i] : 0;

        int p = left + top - top_left;
        int dist_left = abs(p - left);
        int dist_top = abs(p - top);
        int dist_tl = abs(p - top_left);

        int pred;
        if (dist_left <= dist_top && dist_left <= dist_tl) {
            pred = left;
        } else if (dist_top <= dist_tl) {
            pred = top;
        } else {
            pred = top_left;
        }

        cur[i] += pred;
    }
}

void defilter_data(struct image_data* img) {
    if (!img || !img->data) {
        return;
    }

    for (int y = 0; y < (int)img->height; y++) {
        uint8_t filter_type = img->data[y * (img->width * PIXEL_SIZE + 1)];
        for (int x = 0; x < (int)img->width; x++) {
            switch (filter_type) {
                case FILTER_NONE:
                    break;
                case FILTER_SUB:
                    defilter_sub(img, x, y);
                    break;
                case FILTER_UP:
                    defilter_up(img, x, y);
                    break;
                case FILTER_AVERAGE:
                    defilter_average(img, x, y);
                    break;
                case FILTER_PAETH:
                    defilter_paeth(img, x, y);
                    break;
                default:
                    printf("Unknown filter type: %d\n", filter_type);
                    fflush(stdout);
                    exit_error_while_reading_file();
            }
        }
    }
}

void refilter_sub(const struct image_data* img, uint8_t* buff, int x, int y) {
    if (x == 0) {
        return;
    }

    int pos = get_pixel_pos(img, x, y);
    int left_pos = get_left_pixel_pos(img, x, y);

    if (pos < 0 || left_pos < 0)
        return;

    uint8_t* cur_buff = buff + pos;
    uint8_t* cur = img->data + pos;
    uint8_t* left = img->data + left_pos;

    for (int i = 0; i < PIXEL_SIZE; i++) {
        cur_buff[i] = cur[i] - left[i];
    }
}

void refilter_up(const struct image_data* img, uint8_t* buff, int x, int y) {
    if (y == 0) {
        return;
    }

    int pos = get_pixel_pos(img, x, y);
    int top_pos = get_top_pixel_pos(img, x, y);

    if (pos < 0 || top_pos < 0)
        return;

    uint8_t* cur = buff + pos;
    uint8_t* top = img->data + top_pos;

    for (int i = 0; i < PIXEL_SIZE; i++) {
        cur[i] -= top[i];
    }
}

void refilter_average(const struct image_data* img, uint8_t* buff, int x, int y) {
    int pos = get_pixel_pos(img, x, y);
    int left_pos = get_left_pixel_pos(img, x, y);
    int top_pos = get_top_pixel_pos(img, x, y);

    if (pos < 0) {
        return;
    }

    uint8_t* cur = buff + pos;

    for (int i = 0; i < PIXEL_SIZE; i++) {
        uint8_t l = (left_pos >= 0) ? img->data[left_pos + i] : 0;
        uint8_t t = (top_pos >= 0) ? img->data[top_pos + i] : 0;
        cur[i] -= ((l + t) / 2);
    }
}

void refilter_paeth(const struct image_data* img, uint8_t* buff, int x, int y) {
    int pos = get_pixel_pos(img, x, y);
    int left_pos = get_left_pixel_pos(img, x, y);
    int top_pos = get_top_pixel_pos(img, x, y);
    int top_left_pos = get_top_left_pixel_pos(img, x, y);

    if (pos < 0) {
        return;
    }

    uint8_t* cur = buff + pos;

    for (int i = 0; i < PIXEL_SIZE; i++) {
        int left = (left_pos >= 0) ? img->data[left_pos + i] : 0;
        int top = (top_pos >= 0) ? img->data[top_pos + i] : 0;
        int top_left = (top_left_pos >= 0) ? img->data[top_left_pos + i] : 0;

        int p = left + top - top_left;
        int dist_left = abs(p - left);
        int dist_top = abs(p - top);
        int dist_tl = abs(p - top_left);

        int pred;
        if (dist_left <= dist_top && dist_left <= dist_tl) {
            pred = left;
        } else if (dist_top <= dist_tl) {
            pred = top;
        } else {
            pred = top_left;
        }

        cur[i] -= pred;
    }
}

void refilter_data(const struct image_data* img, uint8_t* buff) {
    if (!img || !img->data) {
        exit_error_message("Invalid image data");
    }

    if (!buff) {
        exit_memory_allocation_error();
    }

    buff = memcpy(buff, img->data, img->length * sizeof(uint8_t));

    for (int y = 0; y < (int)img->height; y++) {
        uint8_t filter_type = img->data[y * (img->width * PIXEL_SIZE + 1)];

        for (int x = 0; x < (int)img->width; x++) {
            switch (filter_type) {
                case FILTER_NONE:
                    break;
                case FILTER_SUB:
                    refilter_sub(img, buff, x, y);
                    break;
                case FILTER_UP:
                    refilter_up(img, buff, x, y);
                    break;
                case FILTER_AVERAGE:
                    refilter_average(img, buff, x, y);
                    break;
                case FILTER_PAETH:
                    refilter_paeth(img, buff, x, y);
                    break;
                default:
                    printf("Unknown filter type: %d\n", filter_type);
                    fflush(stdout);
                    exit_error_while_reading_file();
            }
        }
    }
}
