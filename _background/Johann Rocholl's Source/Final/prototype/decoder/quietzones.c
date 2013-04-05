#include "quietzones.h"
#include <stdio.h>
#include "minmax.h"
#include "scanlines.h"
#include "simulator.h"
#include "points.h"
#include "corners.h"


int find_quiet_zones(IplImage *input, IplImage *threshold,
					 int bars_count, Bar *bars,
					 int start, int stop, int step,
					 QuietZone *quietzones, IplImage *debug)
{
	int w = input->width - 1;
	int offsets[input->height];
	int whites[input->height];
	int widths[input->height];
	int found = 0;
	// Background brightness detector.
	for (int index = start; index != stop; index += step) {
		Bar *bar = &bars[index];
		if (!bar->dark)
			continue;
		int y1 = bar->top.y;
		int y2 = bar->bottom.y;
		// printf("index=%d y1=%d y2=%d\n", index, y1, y2);
		for (int y = y1; y <= y2; y++) {
			uchar *pixels = (uchar *)
				(input->imageData + input->widthStep * y);
			uchar *thresh = (uchar *)
				(threshold->imageData + threshold->widthStep * y);
			int x0 = bar->trace[y];
			if (x0 < 0 || x0 > w)
				return found;
			int x = x0;
			while (x > x0 - 10 && x < x0 + 10 && x > 0 && x < w &&
				   pixels[x] < thresh[x])
				x += step;
			int l = x;
			while (l > 0 && pixels[l - 1] + 5 > thresh[l - 1])
				l--;
			if (step > 0 && l < x0)
				l = x0;
			if (step > 0)
				offsets[y] = abs(l - x0);
			int r = x;
			while (r < w && pixels[r + 1] + 5 > thresh[r + 1])
				r++;
			if (step < 0 && r > x0)
				r = x0;
			if (step < 0)
				offsets[y] = abs(r - x0);
			widths[y] = r - l + 1;
			int sum = 0;
			for (x = l; x <= r; x++)
				sum += pixels[x];
			whites[y] = sum / widths[y];
		}
		int height = y2 - y1 + 1;
		qsort(&offsets[bar->top.y], height, sizeof(int), compare_int);
		int offset = offsets[bar->top.y + height / 2];
		qsort(&whites[bar->top.y], height, sizeof(int), compare_int);
		int white = whites[bar->top.y + height / 2];
		qsort(&widths[bar->top.y], height, sizeof(int), compare_int);
		int width = widths[bar->top.y + height / 2];
		if (width < input->width / 30)
			continue;
		// printf("found=%d width=%d height=%d white=%d\n",
		//        found, width, height, white);
		int x1 = bar->middle.x;
		int x2 = x1 + step * width;
		// Save quiet zone to results array.
		quietzones[found].bar = index;
		quietzones[found].x1 = x1;
		quietzones[found].x2 = x2;
		quietzones[found].y1 = y1;
		quietzones[found].y2 = y2;
		quietzones[found].white = white;
		quietzones[found].width = width;
		quietzones[found].height = height;
		found++;
		// Draw box.
		// printf("x1=%d x2=%d y1=%d y2=%d width=%d offset=%d step=%d\n",
		//        x1, x2, y1, y2, width, offset, step);
		CvPoint tl = cvPoint(bar->trace[y1] + offset * step, y1);
		CvPoint tr = cvPoint(tl.x + width * step, y1);
		CvPoint bl = cvPoint(bar->trace[y2] + offset * step, y2);
		CvPoint br = cvPoint(bl.x + width * step, y2);
		cvLine(debug, br, tl, cvScalarAll(white), 3, 8, 0);
		cvLine(debug, bl, tr, cvScalarAll(white), 3, 8, 0);
		cvLine(debug, tl, tr, cvScalar(0, 180, 0, 255), 2, 8, 0);
		cvLine(debug, tr, br, cvScalar(0, 180, 0, 255), 2, 8, 0);
		cvLine(debug, br, bl, cvScalar(0, 180, 0, 255), 2, 8, 0);
		cvLine(debug, bl, tl, cvScalar(0, 180, 0, 255), 2, 8, 0);
	}
	return found;
}


float barcode_quality(int bars_count, Bar *bars,
					  QuietZone *left_quietzone,
					  QuietZone *right_quietzone, CvPoint corners[4])
{
	float quality = 0.0;
	int first = left_quietzone->bar;
	int last = right_quietzone->bar;
	robust_corners(last - first + 1, &bars[first], corners, NULL);
	for (int index = first; index <= last; index++) {
		Bar *bar = &bars[index];
		float top_error = point_line_distance(corners[0].x, corners[0].y,
											  corners[1].x, corners[1].y,
											  bar->top.x, bar->top.y);
		if (top_error < 25.0)
			quality += 5.0 - top_error;
		if (top_error > 25.0 && (index == first || index == last))
			return 0.0;
		float bottom_error = point_line_distance
			(corners[3].x, corners[3].y,
			 corners[2].x, corners[2].y,
			 bar->bottom.x, bar->bottom.y);
		if (bottom_error < 25.0)
			quality += 5.0 - bottom_error;
		if (bottom_error > 40.0 && (index == first || index == last))
			return 0.0;
	}
	quality += left_quietzone->white;
	quality += right_quietzone->white;
	quality += min(left_quietzone->width, 10);
	quality += min(right_quietzone->width, 10);
	return quality;
}


void find_barcode(IplImage *input, IplImage *threshold,
				  int bars_count, Bar *bars,
				  CvPoint corners[4], IplImage *debug)
{
	int half = bars_count / 2;
	QuietZone left_quietzones[half];
	QuietZone right_quietzones[half];
	int left_count = find_quiet_zones(input, threshold,
									  bars_count, bars, half, 0, -1,
									  left_quietzones, debug);
	int right_count = find_quiet_zones(input, threshold,
									   bars_count, bars, half,
									   bars_count - 1, 1,
									   right_quietzones, debug);
	// printf("left_count=%d right_count=%d\n", left_count, right_count);
	int first = 0;
	while (first < bars_count - 1 && !bars[first].dark)
		first++;
	int last = bars_count - 1;
	while (last > 0 && !bars[last].dark)
		last--;
	float best_quality = 0.0;
	CvPoint test_corners[4];
	if (left_count && right_count) {
		for (int l = 0; l < left_count; l++) {
			for (int r = 0; r < right_count; r++) {
				float quality = barcode_quality(bars_count, bars,
												&left_quietzones[l],
												&right_quietzones[r],
												test_corners);
				if (quality > best_quality) {
					best_quality = quality;
					first = left_quietzones[l].bar;
					last = right_quietzones[r].bar;
					memcpy(corners, test_corners, 4 * sizeof(CvPoint));
				}
			}
		}
	}
	// printf("bars_count=%d first=%d last=%d\n", bars_count, first, last);
	draw_bars(bars_count, bars, first, last, debug);
	CvScalar color = cvScalar(255, 0, 0, 255);
	for (int l = 0; l < 4; l++)
		cvLine(debug, corners[l], corners[(l + 1) % 4], color, 3, 8, 0);
}


int estimate_edge(int width, float *samples, int edge, int step)
{
	float white = samples[edge];
	float black = samples[edge];
	int stop = edge + step * width / 3;
	int x;
	for (x = edge; x != stop; x += step) {
		if (samples[x] < white)
			white = samples[x];
		if (samples[x] > black)
			black = samples[x];
	}
	// Find the side of the first bar.
	float threshold = (2 * white + black) / 3;
	int r = width - 1;
	for (x = edge; x > 0 && x < r; x += step)
		if (samples[x] < threshold)
			break;
	int below = x;
	for (x = below; x > 0 && x < r; x += step)
		if (samples[x] > threshold)
			break;
	int middle = x;
	printf("edge=%d middle=%d white=%.3f black=%.3f threshold=%.3f\n",
		   edge, middle, white, black, threshold);
	// Find the peak of the first bar.
	for (x = middle; x > 0 && x < r; x += step)
		if (samples[x + step] < samples[x])
			break;
	int peak = x;
	// Find the foot of first bar.
	threshold = (7 * white + black) / 8;
	for (x = middle; x > 0 && x < r; x -= step)
		if (samples[x] < threshold)
			break;
	int foot = x;
	return (peak + foot) / 2;
}


void estimate_start_stop(int width, float *samples, int *start, int *stop)
{
	*start = estimate_edge(width, samples, *start, 1);
	*stop = estimate_edge(width, samples, *stop, -1);
}
