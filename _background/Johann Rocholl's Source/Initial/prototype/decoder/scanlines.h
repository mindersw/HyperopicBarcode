#pragma once

#include <cv.h>

#include "boolean.h"

#define UNKNOWN 1000000.0

void sample_line_subpixel(IplImage *input,
						  double x1, double y1, double x2, double y2,
						  int width, float *samples);

void scanlines_median(IplImage *input, int scanlines, int squeeze,
					  CvPoint tl, CvPoint tr, CvPoint bl, CvPoint br,
					  int width, float *samples, IplImage *debug);

void derive(int width, const float *input, float *output,
			int start, int stop);

void median(int width, float *samples, int start, int stop,
			float *lowest, float *low, float *middle,
			float *high, float *highest);

void normalize(int width, float *samples, int start, int stop);

void add_ramp(int width, float *samples, int start, int stop,
			  float add_left, float add_right);

void fill_scanline(int width, float *samples, float value,
				   int start, int stop);

void draw_curve(int width, float *samples, float offset_v, float scale_v,
				IplImage *debug, CvScalar color, int thickness);
