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
#include "changes.h"

#define MIN_SAMPLES 400
#define MAX_SAMPLES 1000
#define MAX_KERNEL_WIDTH 51
#define MAX_SECTIONS 96


typedef struct {
	int width;
	float input[MAX_SAMPLES];
	float threshold[MAX_SAMPLES];
	float black[MAX_SAMPLES];
	float white[MAX_SAMPLES];
	float guess[MAX_SAMPLES];
	float blur[MAX_SAMPLES];
	int start;
	int stop;
	double angle;
	double distance;
	int sections[MAX_SECTIONS];
	double sigma;
	int kernel_width;
	float kernel[MAX_KERNEL_WIDTH];
	char unchanged_digits[14];
	char digits[13];
	char checksum;
	Change changes[171];
	int changes_count;
	float error;
} Simulator;


void draw_simulator(Simulator *sim, IplImage *debug, CvFont *font);

void compute_threshold(Simulator *sim);

void compute_kernel(Simulator *sim);

void compute_sections(Simulator *sim);

void initial_guess(Simulator *sim);

void guess_digits(Simulator *sim);

void correct_errors(Simulator *sim, int max_changes, int max_count);

void adjust_sigma(Simulator *sim, int steps, double step_size,
				  double factor);

void printable_digits(Simulator *sim, char *output);

void adjust_start(Simulator *sim, int steps);

void adjust_stop(Simulator *sim, int steps);

void adjust_start_stop(Simulator *sim, int steps);

void printable_digits(Simulator *sim, char *output);

void reset_simulator(Simulator *sim);

