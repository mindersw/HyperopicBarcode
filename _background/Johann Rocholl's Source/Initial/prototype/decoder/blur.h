#pragma once

void compute_gaussian_kernel(int kernel_width, float *kernel,
							 double sigma);

void convolve_1d(int kernel_width, float *kernel,
				 int width, float *input, float *smooth,
				 int start, int stop);
