#pragma once

#include <cv.h>
#include "boolean.h"


typedef struct {
	int *trace;
	CvPoint middle;
	CvPoint top;
	CvPoint bottom;
	bool dark;
} Bar;


int trace_barcode(IplImage *input, IplImage *threshold,
				  CvPoint pt1, CvPoint pt2,
				  int max_traces, int *traces,
				  int max_bars, Bar *bars, IplImage *debug);

void draw_bars(int bars_count, Bar *bars, int first, int last,
			   IplImage *debug);
