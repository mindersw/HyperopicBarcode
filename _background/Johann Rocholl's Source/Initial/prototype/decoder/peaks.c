#include "peaks.h"
#include "boolean.h"
#include "minmax.h"
#include <stdio.h>


int find_peaks(IplImage *input, IplImage *threshold,
			   CvPoint pt1, CvPoint pt2,
			   int max_peaks, Peak *peaks)
{
	int size = input->width + input->height;
	uchar pixels[size];
	int pixel_count = cvSampleLine(input, pt1, pt2, pixels, 4);
	if (pixel_count < 6)
		return 0;
	uchar thresh[size];
	cvSampleLine(threshold, pt1, pt2, thresh, 4);
	int count = 0;
	uchar diff[pixel_count];
	for (int x = 0; x < pixel_count; x++)
		diff[x] = abs(pixels[x] - thresh[x]);
	qsort(diff, pixel_count, sizeof(uchar), compare_unsigned_char);
	int high_difference = diff[95 * pixel_count / 100];
	int minimum_difference = max(2, high_difference / 5);
	printf("high_difference=%d minimum_difference=%d\n",
		   high_difference, minimum_difference);
	int x = 0;
	while (true) {
		// Find the brightest pixel before the next dark area.
		int difference = 0;
		int best_difference = 0;
		int best_x = x;
		while (x < pixel_count - 1) {
			difference = pixels[x] - thresh[x];
			if (-difference >= minimum_difference) break; // Dark area.
			if (difference > best_difference) {
				best_difference = difference;
				best_x = x;
			}
			x++;
		}
		// printf("x=%d count=%d found a bright pixel\n", x, count);
		peaks[count].x = (float) best_x / (pixel_count - 1);
		peaks[count].v = 1.0 - (float) pixels[best_x] / 255.0;
		peaks[count].next = &peaks[count + 1];
		count++;
		if (x >= pixel_count - 1 || count >= max_peaks)
			break;
		// Find the darkest pixel before the next bright area.
		best_difference = 0;
		best_x = x;
		while (x < pixel_count - 1) {
			difference = thresh[x] - pixels[x];
			if (-difference >= minimum_difference) break; // Bright area.
			if (difference > best_difference) {
				best_difference = difference;
				best_x = x;
			}
			x++;
		}
		// printf("x=%d count=%d found a dark pixel\n", x, count);
		peaks[count].x = (float) best_x / (pixel_count - 1);
		peaks[count].v = 1.0 - (float) pixels[best_x] / 255.0;
		peaks[count].next = &peaks[count + 1];
		count++;
		if (x >= pixel_count - 1 || count >= max_peaks)
			break;
	}
	// printf("x=%d left find_peaks loop\n", x);
	peaks[count - 1].next = NULL;
	return count;
}


int count_peaks(Peak *a)
{
	int count = 0;
	while (a) {
		count++;
		a = a->next;
	}
	return count;
}


void draw_peaks(Peak *peaks, CvScalar color, int line_width,
				IplImage *debug)
{
	int width = debug->width - 1;
	int height = debug->height;
	Peak *a = peaks;
	CvPoint pt1 = cvPoint(cvRound(width * a->x),
						  height - 1 - cvRound(height * a->v));
	while (true) {
		CvPoint horizon = cvPoint(cvRound(width * a->x),
								  cvRound(height * 0.5));
		cvLine(debug, pt1, horizon, color, 1, 8, 0);
		a = a->next;
		if (a == NULL)
			break;
		CvPoint pt2 = cvPoint(cvRound(width * a->x),
							  height - 1 - cvRound(height * a->v));
		cvLine(debug, pt1, pt2, color, line_width, 8, 0);
		pt1 = pt2;
	}
}
