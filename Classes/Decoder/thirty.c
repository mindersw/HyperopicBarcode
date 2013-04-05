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

#include "thirty.h"
#include "sections.h"
#include <stdio.h>


int find_peaks_and_valleys(Simulator *sim, int *positions)
{
	int found = 0;
	int best_x;
	int x = sim->start;
	while (x < sim->stop && sim->input[x] <= sim->threshold[x])
		x++;
	while (x < sim->stop) {
		best_x = x;
		while (x < sim->stop && sim->input[x] >= sim->threshold[x]) {
			if (sim->input[x] > sim->input[best_x])
				best_x = x;
			x++;
		}
		positions[found] = best_x;
		found++;
		if (x >= sim->stop)
			break;
		best_x = x;
		while (x < sim->stop && sim->input[x] <= sim->threshold[x]) {
			if (sim->input[x] < sim->input[best_x])
				best_x = x;
			x++;
		}
		positions[found] = best_x;
		found++;
	}
	// printf("found=%d\n", found);
	if (found % 2 == 0)
		found--;				// Last position must be peak.
	// printf("odd found=%d\n", found);
	while (true) {
		int remove_index = -1;
		float smallest_difference = 1000.0;
		for (int index = 0; index < found; index++) {
			x = positions[index];
			float difference = fabs(sim->input[x] - sim->threshold[x]);
			if (difference < smallest_difference) {
				smallest_difference = difference;
				remove_index = index;
			}
		}
		if (remove_index == -1)
			break;
		if (found <= 59 && smallest_difference > 0.005)
			break;
		if (remove_index == found - 1) {	// Right side.
			remove_index--;		// Remove valley and peak on the right.
		} else if (remove_index > 0) {
			int l = positions[remove_index - 1];
			float left_difference =
				fabs(sim->input[l] - sim->threshold[l]);
			int r = positions[remove_index + 1];
			float right_difference =
				fabs(sim->input[r] - sim->threshold[r]);
			if (left_difference < right_difference)
				remove_index--;
		}
		// printf("remove_index=%d\n", remove_index);
		// Remove smallest peak and valley.
		found -= 2;
		for (int index = remove_index; index < found; index++)
			positions[index] = positions[index + 2];
	}
	// printf("final found=%d\n", found);
	return found;
}


int thirty_peaks(Simulator *sim)
{
	int positions[sim->width];
	int count = find_peaks_and_valleys(sim, positions);
	if (count != 59)
		return false;
	int left = (positions[0] + positions[1] + positions[2]) / 3;
	int middle = (positions[27] + positions[28] + positions[29] +
				  positions[30] + positions[31]) / 5;
	int right = (positions[56] + positions[57] + positions[58]) / 3;
	//printf("left=%d middle=%d right=%d angle=%.3f\n", left, middle, right, sim->angle);
	sim->start = left - 15 * (right - left) / 920;
	sim->stop = right + 15 * (right - left) / 920;
	double right_angle = 60 * CV_PI / 180;
	double left_angle = -60 * CV_PI / 180;
	double inside = (double) (sim->stop - sim->start);
	for (int iter = 0; iter < 20; iter++) {
		sim->angle = (right_angle + left_angle) / 2.0;
		double q_u = q(sim->distance, sim->angle, 0.0);
		int attempt = sim->start + cvRound(inside * q_u);
		/*
		printf("left_angle=%.3f angle=%.3f right_angle=%.3f attempt=%d\n",
			   left_angle * 180 / CV_PI, sim->angle * 180 / CV_PI,
			   right_angle * 180 / CV_PI, attempt);
		 */
		if (attempt == middle)
			break;
		if (attempt < middle) {
			left_angle = sim->angle;
		} else {
			right_angle = sim->angle;
		}
	}
	return true;
}
