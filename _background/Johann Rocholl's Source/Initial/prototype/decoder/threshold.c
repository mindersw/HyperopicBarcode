#include "threshold.h"


void adaptive_threshold(IplImage *input, IplImage *output, int radius)
{
	for (int y = 0; y < input->height; y++) {
		uchar *input_pixels = (uchar *)
			(input->imageData + input->widthStep * y);
		uchar *output_pixels = (uchar *)
			(output->imageData + output->widthStep * y);
		int sum = 0;
		for (int x = 0; x < radius; x++)
			sum += input_pixels[x];
		for (int x = 0; x < input->width; x++) {
			int l = x - radius;
			if (l >= 0)
				sum -= input_pixels[l];
			else
				l = -1;
			int r = x + radius;
			if (r < input->width)
				sum += input_pixels[r];
			else
				r = input->width;
			output_pixels[x] = sum / (r - l);
		}
	}
}
