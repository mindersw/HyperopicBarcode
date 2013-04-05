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

#include "peaks.h"
#include "boolean.h"
#include "minmax.h"
#include <stdio.h>

#include "imgproc_c.h"


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
	//printf("high_difference=%d minimum_difference=%d\n", high_difference, minimum_difference);
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
