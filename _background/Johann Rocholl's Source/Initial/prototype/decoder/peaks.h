#pragma once

#include <cv.h>


typedef struct _Peak {
	float x;
	float v;
	struct _Peak *next;
} Peak;


int find_peaks(IplImage *input, IplImage *threshold,
			   CvPoint pt1, CvPoint pt2,
			   int max_peaks, Peak *peaks);

void draw_peaks(Peak *peaks, CvScalar color, int line_width,
				IplImage *debug);
