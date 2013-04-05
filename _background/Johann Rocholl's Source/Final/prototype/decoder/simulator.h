#pragma once

#include <cv.h>
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
