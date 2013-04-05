#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cv.h>

#include "roi.h"
#include "dir8.h"
#include "minmax.h"

#define MIN_GRADIENT_START 30
#define MIN_GRADIENT_CONTINUE 20
#define MIN_VOTES 20
#define MIN_WIDTH 6
#define MIN_LINES 3


void find_line_range(int dir16, int count, const uchar *buffer,
					 int *best_left, int *best_right, int *best_quality)
{
	*best_left = 0;
	*best_right = 0;
	*best_quality = 0;
	int left;
	int right = -1;
	while (right < count - MIN_WIDTH) {
		left = right + 1;
		while (left < count - MIN_WIDTH) {
			if (dir8_similar(buffer[3 * left], dir16) &&
				buffer[3 * left + 1] >= MIN_VOTES &&
				buffer[3 * left + 2] >= MIN_GRADIENT_START)
				break;
			left++;
		}
		if (left >= count - MIN_WIDTH)
			break;
		right = left;
		int quality = 0;
		while (right < count) {
			if ((!dir8_similar(buffer[3 * right], dir16) ||
				 buffer[3 * right + 1] < MIN_VOTES ||
				 buffer[3 * right + 2] < MIN_GRADIENT_CONTINUE)
				&&				// Skipping one bad block is okay.
				(!dir8_similar(buffer[3 * right + 3], dir16) ||
				 buffer[3 * right + 4] < MIN_VOTES ||
				 buffer[3 * right + 5] < MIN_GRADIENT_CONTINUE))
				break;
			quality += buffer[3 * right + 2];	// Gradient
			if (buffer[3 * right] == dir16)	// Exact same direction
				quality += buffer[3 * right + 2];	// Double quality
			right++;
		}
		right--;
		// Symmetry: require start threshold for right side too.
		while (right > left) {
			if (buffer[3 * right + 2] >= MIN_GRADIENT_START)
				break;
			// printf("right=%d gradient=%d\n", right, buffer[3 * right + 2]);
			quality -= buffer[3 * right + 2];	// Gradient
			if (buffer[3 * right] == dir16)	// Exact same direction
				quality -= buffer[3 * right + 2];	// Double quality
			right--;
		}
		assert(left >= 0);
		assert(left < count);
		assert(right >= 0);
		assert(right < count);
		assert(right >= left);
		if (right - left < MIN_WIDTH)
			continue;
		if (quality > *best_quality) {
			*best_quality = quality;
			*best_left = left;
			*best_right = right;
		}
	}
}


CvPoint interpolate(CvPoint pt1, CvPoint pt2, float u,
					CvPoint offset, int zoom)
{
	float px = (float) pt1.x - (float) offset.x;
	float py = (float) pt1.y - (float) offset.y;
	float dx = (float) pt2.x - (float) pt1.x;
	float dy = (float) pt2.y - (float) pt1.y;
	return cvPoint(1 + zoom / 2 + (int) ((float) zoom * (px + dx * u)),
				   1 + zoom / 2 + (int) ((float) zoom * (py + dy * u)));
}


void find_box(const IplImage *blocks, CvPoint offset, int zoom, int dir16,
			  CvPoint *best_left, CvPoint *best_right, int *best_quality,
			  IplImage *debug)
{
	float a = (float) dir16 / 8.0 * CV_PI;
	int size = blocks->height / 2;
	float spacing =
		fabs(cos(a)) > fabs(sin(a)) ? fabs(cos(a)) : fabs(sin(a));
	int lines = (int) round((float) size / spacing);
	float line_x = (float) size * cos(a) / 2.0;
	float line_y = (float) size * sin(a) / 2.0;
	// printf("dir16=%d spacing=%.3f size=%d line_x=%.3f line_y=%.3f\n",
	//        dir16, spacing, size, line_x, line_y);
	// Scanline data
	int length = 2 * size + 1;
	uchar buffer[3 * length];
	// Results from scanlines
	int lefts[lines];
	int rights[lines];
	int qualities[lines];
	// Process each scanline
	for (int y = 0; y < lines; y++) {
		lefts[y] == 0;
		rights[y] == 0;
		qualities[y] == 0;
		float up = spacing * (float) (y - lines / 2);
		CvPoint pt1 =
			cvPoint((int) round((float) size - sin(a) * up - line_x),
					(int) round((float) size + cos(a) * up - line_y));
		CvPoint pt2 =
			cvPoint((int) round((float) size - sin(a) * up + line_x),
					(int) round((float) size + cos(a) * up + line_y));
		// if (y < 3 || y >= lines - 3)
		//   printf("  y=%d pt1=%d,%d pt2=%d,%d\n", y, pt1.x, pt1.y, pt2.x, pt2.y);
		/* Draw all scan lines in blue
		   if (debug) cvLine(debug,
		   cvPoint(pt1.x * zoom, pt1.y * zoom),
		   cvPoint(pt2.x * zoom, pt2.y * zoom),
		   CV_RGB(0, 0, 255), 1, 8, 0);
		 */
		if (pt1.x < 0 || pt1.x >= blocks->width)
			continue;
		if (pt1.y < 0 || pt1.y >= blocks->height)
			continue;
		if (pt2.x < 0 || pt2.x >= blocks->width)
			continue;
		if (pt2.y < 0 || pt2.y >= blocks->height)
			continue;
		int count = cvSampleLine(blocks, pt1, pt2, buffer, 8);
		find_line_range(dir16, count, buffer,
						&lefts[y], &rights[y], &qualities[y]);
		if (!qualities[y])
			continue;
		if (rights[y] - lefts[y] + 1 < MIN_WIDTH)
			continue;
		float c = (float) (count - 1);
		CvPoint l =
			interpolate(pt1, pt2, (float) lefts[y] / c, offset, zoom);
		CvPoint r =
			interpolate(pt1, pt2, (float) rights[y] / c, offset, zoom);
		if (debug)
			cvLine(debug, l, r, cvScalar(0, 0, 255, 255), 1, 8, 0);
	}
	*best_quality = 0;
	int best_middle = 0;
	int best_l = 0;
	int best_r = 0;
	int best_count = 0;
	int first = 0;
	int last = -1;
	while (last < lines) {
		first = last + 1;
		while (first < lines && !qualities[first])
			first++;
		last = first;
		int quality = 0;
		while (last < lines && qualities[last]) {
			if (rights[last + 1] < lefts[last])
				break;
			if (lefts[last + 1] > rights[last])
				break;
			quality += qualities[last];
			last++;
		}
		int count = last - first;
		int middle = (first + last) / 2;
		if (count < MIN_LINES)
			continue;
		last--;
		if (quality > *best_quality) {
			*best_quality = quality;
			best_count = count;
			best_middle = middle;
			qsort(&lefts[first], count, sizeof(int), compare_int);
			best_l = lefts[middle] - 3;
			qsort(&rights[first], count, sizeof(int), compare_int);
			best_r = rights[middle] + 3;
		}
	}
	float up = spacing * (float) (best_middle - lines / 2);
	CvPoint pt1 = cvPoint((int) round((float) size - sin(a) * up - line_x),
						  (int) round((float) size + cos(a) * up -
									  line_y));
	CvPoint pt2 = cvPoint((int) round((float) size - sin(a) * up + line_x),
						  (int) round((float) size + cos(a) * up +
									  line_y));
	int count = cvSampleLine(blocks, pt1, pt2, buffer, 8);
	float c = (float) (count - 1);
	*best_left = interpolate(pt1, pt2, (float) best_l / c, offset, zoom);
	*best_right = interpolate(pt1, pt2, (float) best_r / c, offset, zoom);
	if (debug)
		cvLine(debug, *best_left, *best_right, cvScalar(0, 0, 255, 255), 3,
			   8, 0);
}


void find_center_line(IplImage *blocks, int zoom,
					  CvPoint *best_left, CvPoint *best_right,
					  IplImage *debug)
{
	int longer =
		blocks->width > blocks->height ? blocks->width : blocks->height;
	CvSize border_size = cvSize(2 * longer, 2 * longer);
	IplImage *blocks_border = cvCreateImage(border_size, IPL_DEPTH_8U, 3);
	CvPoint offset = cvPoint((blocks_border->width - blocks->width) / 2,
							 (blocks_border->height - blocks->height) / 2);
	cvCopyMakeBorder(blocks, blocks_border, offset, IPL_BORDER_CONSTANT,
					 cvScalarAll(0));
	CvPoint left;
	CvPoint right;
	int quality;
	int best_quality = 0;
	int best_dir16 = 0;
	for (int dir16 = 0; dir16 < 8; dir16++) {
		find_box(blocks_border, offset, zoom, dir16, &left, &right,
				 &quality, debug);
		if (quality > best_quality) {
			best_quality = quality;
			best_dir16 = dir16;
			if (left.x <= right.x) {
				*best_left = left;
				*best_right = right;
			} else {
				*best_left = right;
				*best_right = left;
			}
		}
	}
	cvReleaseImage(&blocks_border);
	if (debug)
		cvLine(debug, *best_left, *best_right, cvScalar(255, 0, 0, 255), 3,
			   8, 0);
}
