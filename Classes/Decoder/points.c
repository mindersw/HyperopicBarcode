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

#include "points.h"


int square_distance(CvPoint pt1, CvPoint pt2)
{
	int dx = pt2.x - pt1.x;
	int dy = pt2.y - pt1.y;
	return dx * dx + dy * dy;
}


CvPoint point_on_line(CvPoint pt1, CvPoint pt2, double u)
{
	int dx = pt2.x - pt1.x;
	int dy = pt2.y - pt1.y;
	CvPoint result;
	result.x = pt1.x + cvRound(dx * u);
	result.y = pt1.y + cvRound(dy * u);
	return result;
}


int point_in_image(IplImage *image, CvPoint point)
{
	return (point.x >= 0 && point.x < image->width &&
			point.y >= 0 && point.y < image->height);
}


double point_distance(double x1, double y1, double x2, double y2)
{
	double xd = x2 - x1;
	double yd = y2 - y1;
	return sqrt(xd * xd + yd * yd);
}


double point_line_distance(double x1, double y1,
						   double x2, double y2, double x3, double y3)
{
	double x12 = x2 - x1;
	double y12 = y2 - y1;
	double x13 = x3 - x1;
	double y13 = y3 - y1;
	double u1 = x13 * x12 + y13 * y12;
	double u2 = x12 * x12 + y12 * y12;
	double u = u1 / u2;
	double xu = x1 + u * x12;
	double yu = y1 + u * y12;
	double xd = xu - x3;
	double yd = yu - y3;
	double d = sqrt(xd * xd + yd * yd);
	// printf("p1=%.1f,%.1f p2=%.1f,%.1f p3=%.1f,%.1f ", x1, y1, x2, y2, x3, y3);
	// printf("u=%.3f pu=%.1f,%.1f d=%.4f\n", u, xu, yu, d);
	return d;
}
