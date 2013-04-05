#pragma once

#include <cv.h>
#include "boolean.h"

bool decode(const IplImage* red, char* digits, IplImage* debug);
