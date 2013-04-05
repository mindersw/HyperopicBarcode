#pragma once

#include <cv.h>
#include <sys/time.h>

#include "boolean.h"

typedef struct {
	IplImage *red;
	IplImage *threshold;
	IplImage *smooth;
	IplImage *sobel_h;
	IplImage *sobel_v;
	IplImage *gradients;
	IplImage *blocks;
	IplImage *graph;
	IplImage *r;
	IplImage *g;
	IplImage *b;
	IplImage *rgb;
	IplImage *debug;
	CvFont font;
	CvPoint corners[4];
	char digits[14];
	struct timeval started;
} AppData;

void remove_laser(IplImage *red, int top, int height);

void null_images(AppData *data);

void create_images(AppData *data, CvSize size, int graph_columns);

void release_images(AppData *data);

void debug_image(AppData *data, IplImage *image);

bool decode(AppData *data, double angle_of_view,
			const char *debug, const void *userdata,
			void (*progress_callback) (const void *, float));
