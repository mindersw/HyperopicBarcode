#pragma once

#include <cv.h>

void find_box(const IplImage *blocks, CvPoint offset, int zoom, int dir16,
			  CvPoint *best_left, CvPoint *best_right, int *best_quality,
			  IplImage *debug);

void find_center_line(IplImage *blocks, int zoom,
					  CvPoint *best_left, CvPoint *best_right,
					  IplImage *debug);
