#include <stdint.h>
#include <stdio.h>

extern int blur(uint8_t* src, uint8_t* dst, int width, int height);

int blur_data(uint8_t* src, uint8_t* dst, int width, int height) {
    blur(src, dst, width, height);

    return 0;
}