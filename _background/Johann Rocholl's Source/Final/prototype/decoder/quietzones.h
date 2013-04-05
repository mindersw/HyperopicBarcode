#include <cv.h>
#include "trace.h"

typedef struct {
	int bar;
	int x1;
	int x2;
	int y1;
	int y2;
	int white;
	int width;
	int height;
} QuietZone;


void find_barcode(IplImage *input, IplImage *threshold,
				  int bars_count, Bar *bars,
				  CvPoint corners[4], IplImage *debug);

void estimate_start_stop(int width, float *samples, int *start, int *stop);
