#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include <Arduino.h>

// Function declarations
float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, 
                       float *dest, uint8_t dest_rows, uint8_t dest_cols);
float cubicInterpolate(float p[], float x);
float bicubicInterpolate(float p[], float x, float y);

#endif // INTERPOLATION_H
