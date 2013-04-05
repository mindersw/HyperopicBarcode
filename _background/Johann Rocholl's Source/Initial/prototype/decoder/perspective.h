#pragma once

#include <cv.h>

double perspective_distance(double angle_of_view, CvSize size,
							CvPoint pt1, CvPoint pt2);

double perspective_angle(CvPoint tl, CvPoint tr, CvPoint bl, CvPoint br,
						 double distance);
