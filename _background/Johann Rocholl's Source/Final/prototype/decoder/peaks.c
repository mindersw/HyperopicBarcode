#include "peaks.h"
#include "boolean.h"
#include <stdio.h>


int find_peaks(IplImage *input, IplImage *threshold,
			   CvPoint pt1, CvPoint pt2, Peak *peaks)
{
	int size = input->width + input->height;
	uchar pixels[size];
	int pixel_count = cvSampleLine(input, pt1, pt2, pixels, 4);
	if (pixel_count < 6)
		return 0;
	/*
	   uchar thresh[size];
	   cvSampleLine(threshold, pt1, pt2, thresh, 4);
	 */
	int count = 0;
	int x = 0;
	// while (x < pixel_count && pixels[x] <= pixels[x - 1]) x++;
	while (true) {
		// Find a bright pixel.
		while (x < pixel_count - 1 && pixels[x + 1] >= pixels[x])
			x++;
		// printf("x=%d found a bright pixel\n", x);
		peaks[count].x = (float) x / (pixel_count - 1);
		peaks[count].v = 1.0 - (float) pixels[x] / 255.0;
		peaks[count].next = &peaks[count + 1];
		count++;
		if (x >= pixel_count - 1)
			break;
		// Find a dark pixel.
		while (x < pixel_count - 1 && pixels[x + 1] <= pixels[x])
			x++;
		// printf("x=%d found a dark pixel\n", x);
		peaks[count].x = (float) x / (pixel_count - 1);
		peaks[count].v = 1.0 - (float) pixels[x] / 255.0;
		peaks[count].next = &peaks[count + 1];
		count++;
		if (x >= pixel_count - 1)
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


Peak *find_smallest_step(const Peak *peaks)
{
	Peak *smallest_b = NULL;
	float smallest_step = 1000000.0;
	Peak *a = NULL;
	Peak *b = (Peak *) &peaks[0];
	Peak *c = b->next;
	Peak *d = c->next;
	Peak *e = d->next;
	Peak *f = e->next;
	// int counter = 5;
	while (f->next) {
		a = b;
		b = c;
		c = d;
		d = e;
		e = f;
		f = f->next;
		// counter++;
		// printf("counter=%d a=%.4f b=%.4f c=%.4f d=%.4f e=%.4f f=%.4f\n",
		//        counter, a->v, b->v, c->v, d->v, e->v, f->v);
		assert(a->v != b->v);
		if (a->v > b->v) {
			// High values at a, c, e. Low values at b, d, f.
			assert(b->v < c->v);
			assert(c->v > d->v);
			assert(d->v < e->v);
			assert(e->v > f->v);
			// Don't delete c or d if one of them is at the peak.
			if (c->v > a->v && c->v > e->v)
				continue;
			if (d->v < b->v && d->v < f->v)
				continue;
			if (b->v >= e->v)
				continue;
		} else {
			// Low values at a, c, e. High values at b, d, f.
			assert(b->v > c->v);
			assert(c->v < d->v);
			assert(d->v > e->v);
			assert(e->v < f->v);
			// Don't delete c or d if one of them is at the peak.
			if (c->v < a->v && c->v < e->v)
				continue;
			if (d->v > b->v && d->v > f->v)
				continue;
			if (b->v <= e->v)
				continue;
		}
		float step = fabs(d->v - c->v) + (d->x - c->x);
		if (step < smallest_step) {
			smallest_step = step;
			smallest_b = b;
		}
	}
	return smallest_b;
}


void draw_peaks(Peak *peaks, IplImage *debug, CvScalar color,
				int line_width)
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


int best_peaks(Peak *peaks, int max_count, IplImage *debug)
{
	int count = count_peaks(peaks);
	while (count > max_count) {
		// draw_peaks(peaks, debug, cvScalar(255, count, count, 255), 1);
		Peak *a = find_smallest_step(peaks);
		if (a == NULL)
			return count;
		// printf("a->x=%.3f b=%d c=%d d=%d\n", a->x,
		//        a->next, a->next->next, a->next->next->next);
		a->next = a->next->next->next;
		count -= 2;
	}
	// draw_peaks(peaks, debug, cvScalar(0, 0, 0, 255), 1);
	return count;
}
