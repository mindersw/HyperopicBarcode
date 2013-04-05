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

#include "corners.h"
#include "minmax.h"
#include <stdio.h>

//#define MIN_DISTANCE 6


void pixel_darker(IplImage *image, CvPoint p)
{
	if (p.x < 0 || p.x >= image->width)
		return;
	if (p.y < 0 || p.y >= image->height)
		return;
	uchar *pixels = (uchar *) image->imageData;
	pixels += image->widthStep * p.y;
	pixels += image->nChannels * p.x;
	for (int c = 0; c < image->nChannels; c++)
		if (pixels[c] >= 20)
			pixels[c] -= 20;
		else
			pixels[c] = 0;
}


void fit_line(int points_count, CvPoint *points, float *floats,
			  CvPoint *pt1, CvPoint *pt2, IplImage *debug)
{
	int x1[points_count * points_count];
	int y1[points_count * points_count];
	int x2[points_count * points_count];
	int y2[points_count * points_count];
	int count = 0;
	for (int l = 0; l < points_count - 6/* MIN_DISTANCE */; l++) {
        //printf("fit_line: %d\n", points[l].x);
		//float lx = (float) points[l].x;
		//float ly = (float) points[l].y;
		//float lf = floats[l];
		for (int r = l + 6/* MIN_DISTANCE */; r < points_count; r++) {
			//float rx = (float) points[r].x;
			//float ry = (float) points[r].y;
			float rf = floats[r];
			float dx = points[r].x - points[l].x;
			float dy = points[r].y - points[l].y;
			float df = rf - floats[l];
			x1[count] = cvRound(points[l].x - floats[l] * dx / df);
			y1[count] = cvRound(points[l].y - floats[l] * dy / df);
			x2[count] = cvRound(points[r].x + (1.0 - rf) * dx / df);
			y2[count] = cvRound(points[r].y + (1.0 - rf) * dy / df);
			if (debug) {
				pixel_darker(debug, cvPoint(x1[count], y1[count]));
				pixel_darker(debug, cvPoint(x2[count], y2[count]));
			}
			count++;
		}
	}
	qsort(x1, count, sizeof(int), compare_int);
	qsort(x2, count, sizeof(int), compare_int);
	qsort(y1, count, sizeof(int), compare_int);
	qsort(y2, count, sizeof(int), compare_int);
	*pt1 = cvPoint(x1[count / 2], y1[count / 2]);
	*pt2 = cvPoint(x2[count / 2], y2[count / 2]);
}


void robust_corners(int bars_count, Bar *bars, CvPoint *corners,
					IplImage *debug)
{
	int first_x = bars[0].middle.x;
	int last_x = bars[bars_count - 1].middle.x;
	int width = last_x - first_x;
	float floats[bars_count];
	CvPoint tops[bars_count];
	CvPoint bottoms[bars_count];
	for (int i = 0; i < bars_count; i++) {
		tops[i] = bars[i].top;
		bottoms[i] = bars[i].bottom;
		floats[i] = 0.1 + 0.8 * (bars[i].middle.x - first_x) / width;
		// printf("middle=%d,%d %.6f\n",
		// bars[i].middle.x, bars[i].middle.y, floats[i]);
	}
	fit_line(bars_count, tops, floats, &corners[0], &corners[1], debug);
	fit_line(bars_count, bottoms, floats, &corners[3], &corners[2], debug);
}
