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

#include <stdio.h>

#include "scanlines.h"
#include "minmax.h"


// Bilinear subpixel sampling.
void sample_line_subpixel(IplImage *input, double x1, double y1, double x2,
						  double y2, int width, float *samples)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	int fx = cvRound(x1);
	if (fx < 0)
		fx = 0;
	if (fx > input->width - 1)
		fx = input->width - 1;
	int fy = cvRound(y1);
	if (fy < 0)
		fy = 0;
	if (fy > input->height - 1)
		fy = input->height - 1;
	uchar *pixel = (uchar *) input->imageData;
	pixel += input->nChannels * fx;
	pixel += input->widthStep * fy;
	double top_left = pixel[0] / 255.0;
	double top_right = pixel[0] / 255.0;
	double bottom_right = pixel[0] / 255.0;
	double bottom_left = pixel[0] / 255.0;
	int sx = -1;
	int sy = -1;
	for (int i = 0; i < width; i++) {
		double part = (float) i / (width - 1);
		double x = x1 + dx * part;
		double y = y1 + dy * part;
		fx = cvFloor(x);
		fy = cvFloor(y);
		if (fx != sx || fy != sy) {
			if (fx >= 0 && fx <= input->width - 2 &&
				fy >= 0 && fy <= input->height - 2) {
				pixel = (uchar *) input->imageData;
				pixel += input->nChannels * fx;
				pixel += input->widthStep * fy;
				top_left = pixel[0] / 255.0;
				pixel += input->nChannels;	// One pixel column to the right.
				top_right = pixel[0] / 255.0;
				pixel += input->widthStep;	// One pixel row down.
				bottom_right = pixel[0] / 255.0;
				pixel -= input->nChannels;	// One pixel column to the left.
				bottom_left = pixel[0] / 255.0;
			}
			sx = fx;
			sy = fy;
		}
		double rx = x - fx;
		double ry = y - fy;
		// printf("i=%d part=%.4f x=%.4f y=%.4f fx=%d fy=%d rx=%.4f ry=%.4f\n",
		//        i, part, x, y, fx, fy, rx, ry);
		double left = (1.0 - ry) * top_left + ry * bottom_left;
		double right = (1.0 - ry) * top_right + ry * bottom_right;
		double center = (1.0 - rx) * left + rx * right;
		samples[i] = sqrt(1.0 - center);	// Black bars are peaks.
	}
}


void scanlines_median(IplImage *input, int scanlines, int squeeze,
					  CvPoint tl, CvPoint tr, CvPoint bl, CvPoint br,
					  int width, float *samples, IplImage *debug)
{
	double div = (double) scanlines + 1 + squeeze * 2;
	float scanline_samples[scanlines][width];
	for (int i = 0; i < scanlines; i++) {
		double x1 =
			(tl.x * (squeeze + i + 1) +
			 bl.x * (squeeze + scanlines - i)) / div;
		double y1 =
			(tl.y * (squeeze + i + 1) +
			 bl.y * (squeeze + scanlines - i)) / div;
		double x2 =
			(tr.x * (squeeze + i + 1) +
			 br.x * (squeeze + scanlines - i)) / div;
		double y2 =
			(tr.y * (squeeze + i + 1) +
			 br.y * (squeeze + scanlines - i)) / div;
		CvPoint pt1 = cvPoint(cvRound(x1), cvRound(y1));
		CvPoint pt2 = cvPoint(cvRound(x2), cvRound(y2));
		if (debug && i % 2 == 1)
			cvLine(debug, pt1, pt2, cvScalar(255, 0, 0, 255), 1, 8, 0);
		sample_line_subpixel(input, x1, y1, x2, y2, width,
							 scanline_samples[i]);
	}
	float column[scanlines];
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < scanlines; y++)
			column[y] = scanline_samples[y][x];
		qsort(column, scanlines, sizeof(float), compare_float);
		samples[x] = column[scanlines / 2];
		// if (x < 100) printf("%.3f ", samples[x]);
	}
	// printf("\n");
}


void derive(int width, const float *input, float *output,
			int start, int stop)
{
	if (start < 1)
		start = 1;
	if (stop > width - 1)
		stop = width - 1;
	for (int x = start; x < stop; x++) {
		if (input[x - 1] == UNKNOWN)
			continue;
		if (input[x + 1] == UNKNOWN)
			continue;
		output[x] = input[x + 1] - input[x - 1];
	}
}


void median(int width, float *samples, int start, int stop,
			float *lowest, float *low, float *middle,
			float *high, float *highest)
{
	int inside = stop - start;
	float sorted[inside];
	memcpy(sorted, &samples[start], sizeof(float) * inside);
	qsort(sorted, inside, sizeof(float), compare_float);
	if (lowest)
		*lowest = sorted[0];
	if (low)
		*low = sorted[inside * 5 / 100];
	if (middle)
		*middle = sorted[inside * 50 / 100];
	if (high)
		*high = sorted[inside * 95 / 100];
	if (highest)
		*highest = sorted[inside - 1];
}


void normalize(int width, float *samples, int start, int stop)
{
	float low;
	float high;
	median(width, samples, max(0, start - 50), min(width - 1, stop + 50),
		   NULL, &low, NULL, &high, NULL);
	float middle = (low + high) / 2.0;
	float amplitude = high - low;
	for (int x = 0; x < width; x++) {
		samples[x] -= middle;
		samples[x] /= amplitude;
		samples[x] += 0.5;
	}
}


void add_ramp(int width, float *samples, int start, int stop,
			  float add_left, float add_right)
{
	int inside = stop - start;
	for (int x = 0; x < width; x++)
		samples[x] +=
			(add_left * (stop - x) + add_right * (x - start)) / inside;
}


void fill_scanline(int width, float *samples, float value, int start,
				   int stop)
{
	for (int x = start; x < stop; x++)
		samples[x] = value;
}


void draw_curve(int width, float *samples, float offset_v, float scale_v,
				IplImage *debug, CvScalar color, int thickness)
{
	CvPoint previous;
	previous.x = -1;
	for (int x = 0; x < width; x++) {
		if (samples[x] == UNKNOWN) {
			previous.x = -1;
			continue;
		}
		double px = (double) x / width;
		double py = offset_v + scale_v * samples[x];
		CvPoint point = cvPoint(cvRound(debug->width * px),
								cvRound(debug->height *
										(0.9 - 0.85 * py)));
		if (previous.x >= 0)
			cvLine(debug, previous, point, color, thickness, CV_AA, 0);
		previous = point;
	}
}
