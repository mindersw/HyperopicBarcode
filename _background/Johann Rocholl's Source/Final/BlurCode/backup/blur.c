#include <math.h>
#include <stdio.h>
#include "blur.h"
#include "minmax.h"


void compute_gaussian_kernel(Kernel* kernel, double sigma)
{
  double half = (double) (kernel_width - 1) / 2.0;
  double sum = 0;
  for (int x = 0; x < kernel_width; x++) {
    double squarable = ((double) x - half) / sigma;
	double value = exp(-0.5 * squarable * squarable);
    kernel[x] = value;
	sum += value;
  }
  // Normalize kernel values.
  for (int x = 0; x < kernel_width; x++) {
	kernel[x] /= sum;
    // printf("%.4f ", kernel[x]);
  }
  // printf("\n");
}


void convolve_1d(Kernel* kernel,
				 int width, float* input, float* smooth,
				 int start, int stop)
{
  int half = KERNEL_WIDTH / 2;
  if (start < half) start = half;
  if (stop > width - half) stop = width - half;
  for (int x = start; x < stop; x++) {
	double sum = 0.0;
	for (int k = 0; k < kernel_width; k++) {
	  sum += kernel[k] * input[x + k - half];
	}
	smooth[x] = sum;
  }
}


