#include <cv.h>

#define MAX_SECTIONS 96

double p(double distance, double angle, double u);

double q(double distance, double angle, double u);

void calculate_sections(int count, int *sections,
						int start, int stop, double angle,
						double distance);

void draw_sections(int count, int *sections, int width,
				   IplImage *debug, CvScalar color, int thickness);
