#pragma once

#include <cv.h>
#include "boolean.h"

int square_distance(CvPoint pt1, CvPoint pt2);

CvPoint point_on_line(CvPoint pt1, CvPoint pt2, double u);

bool point_in_image(IplImage *image, CvPoint point);

double point_distance(double x1, double y1, double x2, double y2);

double point_line_distance(double x1, double y1,
						   double x2, double y2, double x3, double y3);
