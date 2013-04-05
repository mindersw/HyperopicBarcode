#include "trace.h"
#include "boolean.h"
#include "minmax.h"
#include "peaks.h"
#include <stdio.h>

#define HISTORY 8


void peak_to_trace(IplImage *input, CvPoint p, bool dark, int y_step,
				   int *trace, IplImage *debug)
{
	// printf("trace_peak p=%d,%d y_step=%d dark=%d\n",
	//        p.x, p.y, y_step, dark);
	CvScalar color = dark ?
		cvScalar(0, 0, 255, 255) :
		cvScalar(255, 255, 0, 255);
	int l = 0;
	int r = input->width - 1;
	while (p.y > 0 && p.y < input->height - 1) {
		p.y += y_step;
		uchar *pixels =
			(uchar *) (input->imageData + input->widthStep * p.y);
		int best = p.x;
		if (dark) {				// Find darkest x.
			if (p.x > l && pixels[p.x - 1] < pixels[best])
				best = p.x - 1;
			if (p.x < r && pixels[p.x + 1] < pixels[best])
				best = p.x + 1;
		} else {				// Find brightest x.
			if (p.x > l && pixels[p.x - 1] > pixels[best])
				best = p.x - 1;
			if (p.x < r && pixels[p.x + 1] > pixels[best])
				best = p.x + 1;
		}
		p.x = best;
		trace[p.y] = p.x;
		cvCircle(debug, p, 0, color, CV_FILLED, 8, 0);
	}
}


int peaks_to_traces(IplImage *input, CvPoint pt1, CvPoint pt2,
					Peak *peaks, int *traces, IplImage *debug)
{
	int dx = pt2.x - pt1.x;
	int dy = pt2.y - pt1.y;
	int count = 0;
	Peak *a = peaks;
	while (a) {
		CvPoint peak = cvPoint(pt1.x + cvRound(a->x * dx),
							   pt1.y + cvRound(a->x * dy));
		bool dark = count % 2;
		CvScalar color = dark ?
			cvScalar(0, 0, 255, 255) :
			cvScalar(255, 255, 0, 255);
		cvCircle(debug, peak, 1, color, CV_FILLED, 8, 0);
		int *trace = &traces[input->height * count];
		trace[peak.y] = peak.x;
		peak_to_trace(input, peak, dark, -1, trace, debug);
		peak_to_trace(input, peak, dark, 1, trace, debug);
		count++;
		a = a->next;
	}
	return count;
}


bool endpoint(IplImage *input, int *left, int *center, int *right,
			  int y_start, int y_stop, bool dark, CvPoint *result)
{
	int y_step = y_stop < y_start ? -1 : 1;
	int history[HISTORY];
	memset(history, 0, sizeof(history));
	for (int y = y_start; y != y_stop; y += y_step) {
		if (center[y] < left[y] && !dark)
			return false;
		if (center[y] > right[y] && !dark)
			return false;
		uchar *pixels =
			(uchar *) (input->imageData + input->widthStep * y);
		int l = pixels[left[y]];
		int c = pixels[center[y]];
		int r = pixels[right[y]];
		int cl = abs(c - l);
		int cr = abs(c - r);
		history[y % HISTORY] = max(cl, cr);
		int old = history[(y + y_step + HISTORY) % HISTORY];
		int new = history[y % HISTORY];
		if (3 * new < 2 * old) {
			y -= 2 * y_step;
			*result = cvPoint(center[y], y);
			return true;
		}
	}
	*result = cvPoint(center[y_stop], y_stop);
	return true;
}


bool trace_to_bar(IplImage *input, int *left, int *center, int *right,
				  bool dark, Bar *bar)
{
	bar->dark = dark;
	bar->middle = cvPoint(center[input->height / 2], input->height / 2);
	if (!endpoint(input, left, center, right,
				  input->height / 2, 0, dark, &bar->top))
		return false;
	if (!endpoint(input, left, center, right,
				  input->height / 2, input->height - 1, dark,
				  &bar->bottom))
		return false;
	// int dx = bar->top.x - bar->bottom.x;
	// int dy = bar->top.y - bar->bottom.y;
	// bar->length = cvRound(cvSqrt(dx*dx + dy*dy));
	return true;
}


int traces_to_bars(IplImage *input, int traces_count, int *traces,
				   Bar *bars)
{
	int count = 0;
	for (int i = 1; i < traces_count - 1; i++) {
		int *left = &traces[input->height * (i - 1)];
		int *center = &traces[input->height * i];
		int *right = &traces[input->height * (i + 1)];
		bars[count].trace = center;
		if (trace_to_bar(input, left, center, right, i % 2, &bars[count]))
			count++;
	}
	return count;
}


void adjust_outliers(int bars_count, Bar *bars)
{
	for (int i = 1; i < bars_count - 1; i++) {
		Bar *left = &bars[i - 1];
		Bar *bar = &bars[i];
		Bar *right = &bars[i + 1];
		if (bar->top.y < left->top.y && bar->top.y < right->top.y) {
			bar->top.y = min(left->top.y, right->top.y);
			bar->top.x = bar->trace[bar->top.y];
		}
		if (bar->top.y > left->top.y && bar->top.y > right->top.y) {
			bar->top.y = max(left->top.y, right->top.y);
			bar->top.x = bar->trace[bar->top.y];
		}
		if (bar->bottom.y > left->bottom.y &&
			bar->bottom.y > right->bottom.y) {
			bar->bottom.y = max(left->bottom.y, right->bottom.y);
			bar->bottom.x = bar->trace[bar->bottom.y];
		}
		if (bar->bottom.y < left->bottom.y &&
			bar->bottom.y < right->bottom.y) {
			bar->bottom.y = min(left->bottom.y, right->bottom.y);
			bar->bottom.x = bar->trace[bar->bottom.y];
		}
	}
}


void draw_bars(int bars_count, Bar *bars, int first, int last,
			   IplImage *debug)
{
	for (int i = 0; i < bars_count; i++) {
		CvScalar color = bars[i].dark ? cvScalar(0, 0, 255, 255)
			: cvScalar(255, 255, 0, 255);
		if (i < first || i > last)
			color = bars[i].dark ? cvScalar(128, 0, 128, 255)
				: cvScalar(255, 128, 0, 255);
		cvCircle(debug, bars[i].middle, 1, color, CV_FILLED, 8, 0);
		cvLine(debug, bars[i].top, bars[i].middle, color, 1, 8, 0);
		cvCircle(debug, bars[i].top, 2, color, CV_FILLED, 8, 0);
		cvLine(debug, bars[i].bottom, bars[i].middle, color, 1, 8, 0);
		cvCircle(debug, bars[i].bottom, 2, color, CV_FILLED, 8, 0);
	}
}


int trace_barcode(IplImage *input, IplImage *threshold,
				  CvPoint pt1, CvPoint pt2,
				  int max_traces, int *traces,
				  int max_bars, Bar *bars, IplImage *debug)
{
	Peak peaks[max_traces];
	int peaks_count = find_peaks(input, threshold, pt1, pt2,
								 max_traces, peaks);
	// printf("peaks_count=%d\n", peaks_count);
	if (peaks_count < 6)
		return 0;

	int traces_count =
		peaks_to_traces(input, pt1, pt2, peaks, traces, debug);
	// printf("traces_count=%d\n", traces_count);

	int bars_count = traces_to_bars(input, traces_count, traces, bars);
	// printf("bars_count=%d\n", bars_count);
	adjust_outliers(bars_count, bars);
	return bars_count;
}
