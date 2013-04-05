#pragma once

#include <cv.h>

void find_gradients(const IplImage *sobel_h, const IplImage *sobel_v,
					IplImage *gradients);

void vote_blocks(const IplImage *gradients, IplImage *blocks);

void suppress_non_maxima(const IplImage *gradients, IplImage *maxima);
