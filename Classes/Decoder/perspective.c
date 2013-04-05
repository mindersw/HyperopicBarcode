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

#include "perspective.h"
#include "points.h"


double perspective_distance(double angle_of_view, CvSize size,
							CvPoint pt1, CvPoint pt2)
{
	double view_size = point_distance(0, 0, size.width, size.height);
	double barcode_size = point_distance(pt1.x, pt1.y, pt2.x, pt2.y);
	double angle_of_barcode = angle_of_view / view_size * barcode_size;
	return 1.0 / tan(angle_of_barcode / 2.0);
}


double perspective_angle(CvPoint tl, CvPoint tr, CvPoint bl, CvPoint br,
						 double distance)
{
	double lx = (tl.x + bl.x) / 2.0;
	double ly = (tl.y + bl.y) / 2.0;
	double l = (point_line_distance(tl.x, tl.y, tr.x, tr.y, lx, ly) +
				point_line_distance(bl.x, bl.y, br.x, br.y, lx, ly));
	if (l < 5.0)
		return 0.0;
	double rx = (tr.x + br.x) / 2.0;
	double ry = (tr.y + br.y) / 2.0;
	double r = (point_line_distance(tl.x, tl.y, tr.x, tr.y, rx, ry) +
				point_line_distance(bl.x, bl.y, br.x, br.y, rx, ry));
	if (r < 5.0)
		return 0.0;
	double ratio = l / r;
	double loga = 1.2 * log(ratio);
	double angle = atan(loga);
	// printf("distance=%.2f l=%.2f r=%.2f ratio=%.5f loga=%.5f angle=%.2f\n",
	//        distance, l, r, ratio, loga, angle * 180 / CV_PI);
	return angle;
}
