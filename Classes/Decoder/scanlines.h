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

#pragma once

#include "core_c.h"

#include "boolean.h"

#define UNKNOWN 1000000.0

void sample_line_subpixel(IplImage *input,
						  double x1, double y1, double x2, double y2,
						  int width, float *samples);

void scanlines_median(IplImage *input, int scanlines, int squeeze,
					  CvPoint tl, CvPoint tr, CvPoint bl, CvPoint br,
					  int width, float *samples, IplImage *debug);

void derive(int width, const float *input, float *output,
			int start, int stop);

void median(int width, float *samples, int start, int stop,
			float *lowest, float *low, float *middle,
			float *high, float *highest);

void normalize(int width, float *samples, int start, int stop);

void add_ramp(int width, float *samples, int start, int stop,
			  float add_left, float add_right);

void fill_scanline(int width, float *samples, float value,
				   int start, int stop);

void draw_curve(int width, float *samples, float offset_v, float scale_v,
				IplImage *debug, CvScalar color, int thickness);
