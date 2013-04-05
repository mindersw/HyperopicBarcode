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

#include "illumination.h"
#include "minmax.h"

#define SEGMENTS 7


void segment_adjustments(Simulator *sim, int start, int stop,
						 float *adjust_black, float *adjust_white)
{
	int segment_width = stop - start;
	float values[segment_width];
	memcpy(values, &sim->input[start], sizeof(float) * segment_width);
	qsort(values, segment_width, sizeof(float), compare_float);
	float input_black = values[95 * segment_width / 100];
	float input_white = values[5 * segment_width / 100];
	memcpy(values, &sim->blur[start], sizeof(float) * segment_width);
	qsort(values, segment_width, sizeof(float), compare_float);
	float blur_black = values[95 * segment_width / 100];
	float blur_white = values[5 * segment_width / 100];
	*adjust_black = input_black - blur_black;
	*adjust_white = input_white - blur_white;
}


void adjust_light_level(int width, float *level,
						int start, int stop,
						float *adjust, float factor)
{
	float smooth[SEGMENTS + 1];
	smooth[0] = adjust[0];
	smooth[SEGMENTS] = adjust[SEGMENTS];
	for (int s = 1; s < SEGMENTS; s++)
		smooth[s] = (adjust[s - 1] + adjust[s] + adjust[s + 1]) / 3.0;
	int inside = stop - start;
	for (int s = 0; s < SEGMENTS; s++) {
		int x1 = start + inside * s / SEGMENTS;
		int x2 = start + inside * (s + 1) / SEGMENTS;
		int segment_width = x2 - x1;
		for (int x = x1; x < x2; x++) {
			float smooth1 = (x2 - x) * smooth[s];
			float smooth2 = (x - x1) * smooth[s + 1];
			level[x] += factor * (smooth1 + smooth2) / segment_width;
		}
	}
	// Left outside.
	for (int x = 0; x < start; x++)
		level[x] += factor * smooth[0];
	// Right outside.
	for (int x = stop; x < width; x++)
		level[x] += factor * smooth[SEGMENTS];
}


void adjust_light_levels(Simulator *sim, float factor)
{
	float adjust_white[SEGMENTS + 1];
	float adjust_black[SEGMENTS + 1];
	int sim_range = sim->stop - sim->start;
	for (int s = 0; s <= SEGMENTS; s++) {
		int start = sim->start + sim_range * (s - 1) / SEGMENTS;
		int stop = sim->start + sim_range * (s + 1) / SEGMENTS;
		if (start < sim->start) start = sim->start;
		if (stop > sim->stop) stop = sim->stop;
		segment_adjustments(sim, start, stop, &adjust_black[s],
							&adjust_white[s]);
	}
	adjust_light_level(sim->width, sim->black, sim->start, sim->stop,
					   adjust_black, factor);
	adjust_light_level(sim->width, sim->white, sim->start, sim->stop,
					   adjust_white, factor);
}
