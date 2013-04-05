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

#include <math.h>
#include <stdio.h>

#include "sections.h"


double p(double distance, double angle, double u)
{
	double cos_angle = cos(angle);
	double sin_angle = sin(angle);
	return (distance * u * cos_angle) / (distance + u * sin_angle);
}


double q(double distance, double angle, double u)
{
	double cos_angle = cos(angle);
	double sin_angle = sin(angle);
	double p_u = (distance * u * cos_angle) / (distance + u * sin_angle);
	double p_left = (distance * -1 * cos_angle) / (distance - sin_angle);
	double p_right = (distance * 1 * cos_angle) / (distance + sin_angle);
	return (p_u - p_left) / (p_right - p_left);
}


void calculate_sections(int count, int *sections,
						int start, int stop, double angle, double distance)
{
	double cos_angle = cos(angle);
	double sin_angle = sin(angle);
	double p_left = (distance * -1 * cos_angle) / (distance - sin_angle);
	double p_right = (distance * 1 * cos_angle) / (distance + sin_angle);
	double p_range = p_right - p_left;
	double inside = (double) (stop - start);
	for (int i = 0; i < count; i++) {
		double u = -1.0 + 2.0 * i / (count - 1);
		double p_u =
			(distance * u * cos_angle) / (distance + u * sin_angle);
		double q_u = (p_u - p_left) / p_range;
		sections[i] = start + cvRound(q_u * inside);
		// printf("u=%.6f p_u=%.6f q_u=%.6f sections[%d]=%d\n",
		//        u, p_u, q_u, i, sections[i]);
	}
}


void draw_sections(int count, int *sections, int width,
				   IplImage *debug, CvScalar color, int thickness)
{
	for (int i = 0; i <= count; i++) {
		if (i > 3 && i < 45 && (i - 3) % 7)
			continue;
		if (i > 50 && i < 92 && (i - 50) % 7)
			continue;
		double px = (double) sections[i] / width;
		int x = cvRound(debug->width * px);
		CvPoint pt1 = cvPoint(x, 0);
		CvPoint pt2 = cvPoint(x, debug->height - 1);
		cvLine(debug, pt1, pt2, color, thickness, 8, 0);
	}
}
