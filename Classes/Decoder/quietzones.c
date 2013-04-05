//	(c) Copyright 2008-2009 Johann C. Rocholl
//	Portions (c) Copyright 2012 Conor Dearden & Michael LaMorte
//
//  Redistribution and use in source and binary forms, with or without modification, 
//  are permitted provided that the following conditions are met:
//
//      * Redistributions of source code must retain the above copyright notice, 
//  	  this list of conditions and the following disclaimer.
//      * Redistributions in binary form must reproduce the above copyright notice, 
//  	  this list of conditions and the following disclaimer in the documentation 
//  	  and/or other materials provided with the distribution.
//      * The name of the copyright holders may not be used to endorse or promote products
//        derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
//  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,  
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT  
//  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR  
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  
//  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
//  POSSIBILITY OF SUCH DAMAGE.
//

#include "quietzones.h"
#include <stdio.h>
#include "boolean.h"
#include "minmax.h"
#include "scanlines.h"
#include "simulator.h"
#include "points.h"
#include "corners.h"


int measure_quiet_zone(IplImage *input, IplImage *threshold,
						Bar *bar, Bar *next_dark_bar, int step,
						QuietZone *quietzone)
{
	int w = input->width - 1;
	int offsets[input->height];
	int whites[input->height];
	int widths[input->height];
	int y1 = bar->top.y;
	int y2 = bar->bottom.y;
	// printf("index=%d y1=%d y2=%d\n", index, y1, y2);
	for (int y = y1; y <= y2; y++) {
		uchar *pixels = (uchar *)
			(input->imageData + input->widthStep * y);
		uchar *thresh = (uchar *)
			(threshold->imageData + threshold->widthStep * y);
		int x0 = bar->trace[y];
		if (x0 < 0 || x0 > w) {
			printf("aborting because trace[%d]=%d\n", y, x0);
			return false;
		}
		int x = x0;
		int stop_x = x0 + 20 * step;
		if (next_dark_bar) stop_x = (x0 + next_dark_bar->middle.x -
									 bar->middle.x) - 2 * step;
		if (stop_x > w) stop_x = w;
		if (stop_x < 0) stop_x = 0;
		int brightest_x = x;
		int brightest_pixel = pixels[x];
		while (x != stop_x) {
			if (pixels[x] > brightest_pixel) {
				brightest_pixel = pixels[x];
				brightest_x = x;
			}
			x += step;
		}
		x = brightest_x;
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
	if (height < 10) return false;
	quietzone->height = height;
	qsort(&offsets[bar->top.y], height, sizeof(int), compare_int);
	quietzone->offset = offsets[bar->top.y + height / 2];
	qsort(&whites[bar->top.y], height, sizeof(int), compare_int);
	quietzone->white = whites[bar->top.y + height / 2];
	qsort(&widths[bar->top.y], height, sizeof(int), compare_int);
	quietzone->width = widths[bar->top.y + height / 2];
	// printf("found=%d width=%d height=%d white=%d\n",
	//        found, width, height, white);
	quietzone->x1 = bar->middle.x;
	quietzone->x2 = quietzone->x1 + step * quietzone->width;
	// Save quiet zone to results array.
	quietzone->y1 = y1;
	quietzone->y2 = y2;
	return true;
}


int find_quiet_zones(IplImage *input, IplImage *threshold,
					 int bars_count, Bar *bars,
					 int start, int stop, int step,
					 QuietZone *quietzones, IplImage *debug)
{
	//int w = input->width - 1;
	int found = 0;
	// Background brightness detector.
	for (int index = start; index != stop + step; index += step) {
		Bar *bar = &bars[index];
		if (!bar->dark)
			continue;
		int next_dark_index = index + step;
		while (next_dark_index >= 0 && next_dark_index < bars_count &&
			   !bars[next_dark_index].dark)
			next_dark_index += step;
		Bar *next_dark_bar = NULL;
		if (next_dark_index >= 0 &&	next_dark_index < bars_count)
			next_dark_bar = &bars[next_dark_index];
		if (!measure_quiet_zone(input, threshold, bar, next_dark_bar, step,
								&quietzones[found])) {
			printf("could not measure from x=%d\n", bar->middle.x);
			continue;
		}
		if (quietzones[found].width < input->width / 30 && index != stop)
			continue;
		quietzones[found].index = index;
		/*
		printf("x1=%d x2=%d y1=%d y2=%d size=%dx%d offset=%d step=%d\n",
			   quietzones[found].x1, quietzones[found].x2,
			   quietzones[found].y1, quietzones[found].y2,
			   quietzones[found].width, quietzones[found].height,
			   quietzones[found].offset, step);
		*/
		// Draw box.
		int y1 = bar->top.y;
		int y2 = bar->bottom.y;
		int offset = quietzones[found].offset;
		int width = quietzones[found].width;
		CvPoint tl = cvPoint(bar->trace[y1] + offset * step, y1);
		CvPoint tr = cvPoint(tl.x + width * step, y1);
		CvPoint bl = cvPoint(bar->trace[y2] + offset * step, y2);
		CvPoint br = cvPoint(bl.x + width * step, y2);
		cvLine(debug, tl, tr, cvScalar(0, 180, 0, 255), 2, 8, 0);
		cvLine(debug, tr, br, cvScalar(0, 180, 0, 255), 2, 8, 0);
		cvLine(debug, br, bl, cvScalar(0, 180, 0, 255), 2, 8, 0);
		cvLine(debug, bl, tl, cvScalar(0, 180, 0, 255), 2, 8, 0);
		found++;
	}
    
    int qzf = &quietzones[found - 1].width;
    
	if (qzf < input->width / 20)
		quietzones[found - 1].width = input->width / 20;
	return found;
}


int found_two_more_bars(int bars_count, Bar *bars, int first, int last,
						 int n0, int n1, int n2, CvPoint corners[4])
{
	Bar *bar = &bars[n0];
	Bar *next1 = &bars[n1];
	Bar *next2 = &bars[n2];
	float lb = point_distance(bar->top.x, bar->top.y,
							  bar->bottom.x, bar->bottom.y);
	float l1 = point_distance(next1->top.x, next1->top.y,
							  next1->bottom.x, next1->bottom.y);
	if (l1 < lb / 2) return false;
	float l2 = point_distance(next2->top.x, next2->top.y,
							  next2->bottom.x, next2->bottom.y);
	if (l2 < lb / 2) return false;
	int average = (bars[last].middle.x - bars[first].middle.x) /
		(last - first);
	int distance = abs(next2->middle.x - bar->middle.x);
	// printf("average=%d distance=%d\n", average, distance);
	if (distance > 3 * average)
		return false;
	// if (abs(next2->middle.x - )) return false;
	return true;
}


float barcode_quality(int bars_count, Bar *bars,
					  QuietZone *left_quietzone,
					  QuietZone *right_quietzone, CvPoint corners[4])
{
	float quality = 0.0;
	int first = left_quietzone->index;
	int last = right_quietzone->index;
	if (last - first < 10) return 0.0;
	float width = bars[last].middle.x - bars[first].middle.x;
	float half_error = (float) width / (last - first);
	float max_error = 2 * half_error;
	robust_corners(last - first + 1, &bars[first], corners, NULL);
	float top_edge_width = corners[1].x - corners[0].x;
	float top_edge_height = corners[1].y - corners[0].y;
	float bottom_edge_width = corners[2].x - corners[3].x;
	float bottom_edge_height = corners[2].y - corners[3].y;
	for (int index = first; index <= last; index++) {
		Bar *bar = &bars[index];
		float top_edge_y = corners[0].y + top_edge_height *
			(bar->top.x - corners[0].x) / top_edge_width;
		float top_error = top_edge_y - bar->top.y;
		float bottom_edge_y = corners[3].y + bottom_edge_height *
			(bar->bottom.x - corners[3].x) / bottom_edge_width;
		float bottom_error = bar->bottom.y - bottom_edge_y;
		/*
		printf("top=%d,%d top_edge_y=%.1f top_error=%.1f\n",
			   bar->top.x, bar->top.y, top_edge_y, top_error);
		printf("bottom=%d,%d bottom_edge_y=%.1f bottom_error=%.1f\n",
			   bar->bottom.x, bar->bottom.y, bottom_edge_y, bottom_error);
		*/
		// High quality for many matching bars.
		if (fabs(bottom_error) < max_error)
			quality += half_error - fabs(bottom_error);
		if (fabs(top_error) < max_error)
			quality += half_error - fabs(top_error);
		// Bars should not exceed top edge.
		if (top_error > 3.0) quality -= top_error * 2.0;
		// First and last bar must match better.
		if (index == first || index == last) {
			// printf("index=%d top_error=%.1f bottom_error=%.1f max_error=%.1f\n",
			//        index, top_error, bottom_error, max_error);
			if (top_error > max_error) quality -= 100.0;
			if (top_error < -max_error) quality -= 500.0;
			if (bottom_error > 2 * max_error) quality -= 100.0;
			if (bottom_error < -max_error) quality -= 500.0;
		}
	}
	quality += left_quietzone->white;
	quality += right_quietzone->white;
	quality += min(left_quietzone->width, 10);
	quality += min(right_quietzone->width, 10);
	// Lower quality if two more matching bars in left quiet zone.
	if (first >= 2 &&
		found_two_more_bars(bars_count, bars, first, last,
							first, first - 1, first - 2, corners))
		quality /= 3;
	// Lower quality if two more matching bars in right quiet zone.
	if (last <= bars_count - 3 &&
		found_two_more_bars(bars_count, bars, first, last,
							last, last + 1, last + 2, corners))
		quality /= 3;
	return quality;
}


void avoid_internal_quietzone(QuietZone *quietzones, int current,
							  int acceptable_width, float *quality)
{
	int width = 0;
	for (int index = current - 1; index >= 0; index--)
		if (quietzones[index].width > width)
			width = quietzones[index].width;
	if (width > acceptable_width) {
		// float before = *quality;
		(*quality) -= 4 * width;
		// printf("internal quietzone width=%d acceptable_width=%d ",
		//        width, acceptable_width);
		// printf("before=%.1f quality=%.1f\n", before, *quality);
	}
}


void find_barcode(IplImage *input, IplImage *threshold,
				  int bars_count, Bar *bars,
				  CvPoint corners[4], IplImage *debug)
{
    int half;
    if ((fmod((double)bars_count, 2)) == 0.0) {
        half = bars_count / 2;
    } else {
        return;
    }
	QuietZone left_quietzones[half];
	QuietZone right_quietzones[half];
	int left_count = find_quiet_zones(input, threshold, bars_count, bars,
									  half, 0, -1, left_quietzones, debug);
	int right_count = find_quiet_zones(input, threshold, bars_count, bars,
									   half, bars_count -1, 1,
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
				// Estimate quality of barcode between quiet zones.
				float quality = barcode_quality(bars_count, bars,
												&left_quietzones[l],
												&right_quietzones[r],
												test_corners);
				// Lower quality if other quiet zones just inside of these.
				if (l > 0 &&
					left_quietzones[l - 1].index ==
					left_quietzones[l].index - 2)
					quality -= 20;
				if (r > 0 &&
					right_quietzones[r - 1].index ==
					right_quietzones[r].index - 2)
					quality -= 20;
				// Lower quality if wide internal quiet zone.
				float w = input->width / 20;
				avoid_internal_quietzone(left_quietzones, l, w, &quality);
				avoid_internal_quietzone(right_quietzones, r, w, &quality);
				if (quality > best_quality) {
					best_quality = quality;
					//ml commented out: first = left_quietzones[l].index;
					//ml commented out: last = right_quietzones[r].index;
					memcpy(corners, test_corners, 4 * sizeof(CvPoint));
				}
			}
		}
	}
	// printf("bars_count=%d first=%d last=%d\n", bars_count, first, last);
	// draw_bars(bars_count, bars, first, last, debug);
	CvScalar color = cvScalar(255, 0, 0, 255);
	for (int l = 0; l < 4; l++)
		cvLine(debug, corners[l], corners[(l + 1) % 4], color, 3, 8, 0);
}


int estimate_edge(int width, float *samples, int edge, int step)
{
	float white = samples[edge];
	float black = samples[edge];
	int stop = edge + step * width / 5;
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
	//printf("edge=%d middle=%d white=%.3f black=%.3f threshold=%.3f\n", edge, middle, white, black, threshold);
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
