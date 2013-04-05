#pragma once

#define KERNEL_WIDTH 51

typedef struct {
	int width;
	float values[MAX_KERNEL_WIDTH];
} Kernel;

void compute_gaussian_kernel(double sigma, Kernel* kernel);

void convolve_1d(int width, float* input, float* smooth,
				 int start, int stop, Kernel* kernel);
