#pragma once

#include <cv.h>


typedef struct _Peak {
	float x;
	float v;
	struct _Peak *next;
} Peak;


int find_peaks(IplImage *input, IplImage *threshold,
			   CvPoint pt1, CvPoint pt2, Peak *peaks);

int best_peaks(Peak *peaks, int max_count, IplImage *debug);
