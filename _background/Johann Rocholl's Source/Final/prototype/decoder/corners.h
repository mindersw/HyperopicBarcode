#pragma once

#include <cv.h>
#include "trace.h"


void robust_corners(int bars_count, Bar *bars, CvPoint *corners,
					IplImage *debug);
