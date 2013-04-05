#pragma once

extern char L[11];
extern char R[11];
extern char G[11];
extern char LG[21];
extern char FIRST_DIGIT[11];

void encode_bits(int width, float *black, float *white, float *values,
				 int *sections, int count, int bits);

void encode_ean13(int width, float *black, float *white, float *values,
				  int *sections, char *digits);

// void draw_guards(int width, float* values);

// void draw_upc(int* row, int width, char digits[12]);

// void draw_ean(int* row, int width, char digits[13]);
