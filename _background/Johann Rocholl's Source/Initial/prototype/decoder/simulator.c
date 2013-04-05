#include <stdio.h>

#include "simulator.h"
#include "encoder.h"
#include "minmax.h"
#include "scanlines.h"
#include "blur.h"
#include "checksums.h"
#include "sections.h"

#define EXTRA_SAMPLES 20


void reset_simulator(Simulator *sim)
{
	sim->width = MAX_SAMPLES;
	for (int x = 0; x < MAX_SAMPLES; x++) {
		sim->input[x] = 0.5;
		sim->threshold[x] = 0.5;
		sim->black[x] = 1.0;
		sim->white[x] = 0.0;
		sim->guess[x] = 0.5;
	    sim->blur[x] = 0.5;
	}
	sim->start = 0;
	sim->stop = 0;
	sim->angle = 0.0;
	sim->distance = 4.0;
	sim->sections[MAX_SECTIONS];
	sim->sigma = 1.0;
	sim->kernel_width = 0;
	for (int x = 0; x < MAX_KERNEL_WIDTH; x++)
		sim->kernel[x] = 0.0;
	sim->checksum = '0';
	sim->changes_count = 0;
	sim->error = 0.0;
}


double samples_error(Simulator *sim, int start, int stop)
{
	double error = 0.0;
	for (int x = start; x < stop; x++) {
		if (sim->guess[x] == UNKNOWN)
			continue;
		double difference = sim->blur[x] - sim->input[x];
		error += difference * difference;
	}
	return error;
}


double sections_error(Simulator *sim, int start_section, int stop_section)
{
	return samples_error(sim,
						 sim->sections[start_section],
						 sim->sections[stop_section]);
}


double barcode_error(Simulator *sim)
{
	return sections_error(sim, 0, 95);
}


void draw_digit(int width, int x, int y, char *digit,
				IplImage *debug, CvFont *font)
{
	char text[3] = { digit[0] + 48, 0, 0 };
	if (text[0] > '9') {
		text[0] -= 10;
		text[1] = '.';
	}
	CvPoint origin = cvPoint(debug->width * x / width, debug->height - y);
	cvPutText(debug, text, origin, font, cvScalar(0, 0, 0, 255));
	origin.x--;
	cvPutText(debug, text, origin, font, cvScalar(0, 0, 255, 255));
}


void draw_digits(Simulator *sim, char *digits, char *compare, int y,
				 IplImage *debug, CvFont *font)
{
	if (!compare || digits[0] != compare[0])
		draw_digit(sim->width, sim->sections[0] - 40, y,
				   &digits[0], debug, font);
	for (int i = 0; i < 6; i++) {
		if (!compare || digits[1 + i] != compare[1 + i])
			draw_digit(sim->width, sim->sections[6 + i * 7], y,
					   &digits[1 + i], debug, font);
		if (!compare || digits[7 + i] != compare[7 + i])
			draw_digit(sim->width, sim->sections[53 + i * 7], y,
					   &digits[7 + i], debug, font);
	}
	if (compare)
		draw_digit(sim->width, sim->sections[95] + 10, y,
				   &sim->checksum, debug, font);
}


void draw_simulator(Simulator *sim, IplImage *debug, CvFont *font)
{
	float bright = 0.0;
	float dark = 1.0;
	median(sim->width, sim->white, sim->start, sim->stop,
		   NULL, &bright, NULL, NULL, NULL);
	median(sim->width, sim->black, sim->start, sim->stop,
		   NULL, NULL, NULL, &dark, NULL);
	bright -= (dark - bright) / 10;
	float scale = 1.0 / (dark - bright);
	float offset = -bright * scale;
	// printf("bright=%.3f dark=%.3f offset=%.3f scale=%.3f\n",
	//        bright, dark, offset, scale);
	cvSet(debug, cvScalarAll(255), NULL);
	draw_sections(96, sim->sections, sim->width,
				  debug, cvScalar(160, 160, 160, 255), 1);
	draw_curve(sim->width, sim->input, offset, scale,
			   debug, cvScalar(255, 0, 0, 255), 1);
	draw_curve(sim->width, sim->threshold, offset, scale,
			   debug, cvScalar(255, 160, 0, 255), 1);
	draw_curve(sim->width, sim->black, offset, scale,
			   debug, cvScalar(180, 180, 180, 255), 1);
	draw_curve(sim->width, sim->white, offset, scale,
			   debug, cvScalar(180, 180, 180, 255), 1);
	draw_curve(sim->width, sim->guess, offset, scale,
			   debug, cvScalar(160, 160, 255, 255), 1);
	draw_curve(sim->width, sim->blur, offset, scale,
			   debug, cvScalar(0, 0, 255, 255), 1);
	draw_digits(sim, sim->unchanged_digits, sim->digits, 34, debug, font);
	draw_digits(sim, sim->digits, NULL, 4, debug, font);
}


void compute_threshold(Simulator *sim)
{
	int w = sim->width - 1;
	int radius = sim->width / 60;
	float sum = 0.0;
	for (int x = 0; x < radius; x++)
		sum += sim->input[x];
	for (int x = 0; x < sim->width; x++) {
		int l = x - radius;
		if (l < 0)
			l = -1;
		else
			sum -= sim->input[l];
		int r = x + radius;
		if (r > w)
			r = w;
		else
			sum += sim->input[r];
		sim->threshold[x] = sum / (r - l);
		// printf("x=%d input=%.3f threshold=%.3f\n",
		//        x, sim->input[x], sim->threshold[x]);
	}
}


int guess_digit(Simulator *sim, int *sections, int index,
				int choices_count, char *choices)
{
	// printf("guess_digit start=%d stop=%d\n", sections[0], sections[7]);
	int start = sim->changes_count;
	int best_choice = -1;
	double best_error = UNKNOWN;
	for (int choice = 0; choice < choices_count; choice++) {
		encode_bits(sim->width, sim->black, sim->white, sim->guess,
					sections, 7, choices[choice]);
		convolve_1d(sim->kernel_width, sim->kernel,
					sim->width, sim->guess, sim->blur,
					sections[0], sections[7]);
		double error = samples_error(sim, sections[0], sections[7]);
		if (error < best_error) {
			best_choice = choice;
			best_error = error;
		}
		sim->changes[sim->changes_count].error = error;
		sim->changes[sim->changes_count].index = index;
		sim->changes[sim->changes_count].digit = choice;
		sim->changes_count++;
	}
	// Make all changes relative to the best choice.
	for (int i = start; i < sim->changes_count; i++)
		sim->changes[i].error -= sim->changes[start + best_choice].error;
	// Remove the best choice from the changes array.
	if (best_choice < choices_count - 1)
		sim->changes[start + best_choice] =
			sim->changes[start + choices_count - 1];
	sim->changes_count--;
	return best_choice;
}


void guess_digits(Simulator *sim)
{
	// printf("guess_digits...\n");
	sim->changes_count = 0;
	sim->digits[0] = 0;
	for (int i = 0; i < 6; i++) {
		sim->digits[1 + i] =
			guess_digit(sim, &sim->sections[3 + i * 7], 1 + i, i ? 20 : 10,
						LG);
		sim->digits[7 + i] =
			guess_digit(sim, &sim->sections[50 + i * 7], 7 + i, 10, R);
	}
	encode_ean13(sim->width, sim->black, sim->white, sim->guess,
				 sim->sections, sim->digits);
	convolve_1d(sim->kernel_width, sim->kernel,
				sim->width, sim->guess, sim->blur, 0, sim->width);
	sim->digits[0] = ean13_first_digit(sim->digits);
	sim->checksum = ean13_checksum(sim->digits);
	memcpy(sim->unchanged_digits, sim->digits, 13);
}


void correct_errors(Simulator *sim, int max_changes, int max_count)
{
	if (is_valid_barcode(sim->digits))
		return;
	qsort(sim->changes, sim->changes_count, sizeof(Change), compare_float);
	// for (int i = 0; i < 10; i++)
	//   printf("error=%.6f index=%d digit=%d\n",
	//          changes[i].error, changes[i].index, changes[i].digit);
	if (sim->changes_count > max_count)
		sim->changes_count = max_count;
	// Terminate search for valid candidate.
	sim->changes[sim->changes_count].index = -1;
	best_valid_barcode(sim->changes, max_changes, sim->digits);
	encode_ean13(sim->width, sim->black, sim->white, sim->guess,
				 sim->sections, sim->digits);
	convolve_1d(sim->kernel_width, sim->kernel,
				sim->width, sim->guess, sim->blur, 0, sim->width);
	sim->checksum = ean13_checksum(sim->digits);
}


void compute_kernel(Simulator *sim)
{
	// printf("compute_kernel...\n");
	sim->kernel_width = cvRound(6 * sim->sigma) + 1;
	if (sim->kernel_width > MAX_KERNEL_WIDTH)
		sim->kernel_width = MAX_KERNEL_WIDTH;
	if (sim->kernel_width % 2 == 0)
		sim->kernel_width--;
	compute_gaussian_kernel(sim->kernel_width, sim->kernel, sim->sigma);
}


void compute_sections(Simulator *sim)
{
	// printf("compute_sections...\n");
	calculate_sections(96, sim->sections, sim->start, sim->stop,
					   sim->angle, sim->distance);
}


void initial_guess(Simulator *sim)
{
	// printf("initial_guess...\n");
	for (int x = 0; x < sim->width; x++) {
		// sim->guess[x] = sim->input[x];
		sim->guess[x] = (sim->white[x] + sim->black[x]) / 2;
		sim->blur[x] = UNKNOWN;
	}
	// Shoulders.
	for (int x = 0; x < sim->sections[0]; x++)
		sim->guess[x] = sim->white[x];
	for (int x = sim->sections[95]; x < sim->width; x++)
		sim->guess[x] = sim->white[x];
	// Guard bars.
	encode_bits(sim->width, sim->black, sim->white, sim->guess,
				&sim->sections[0], 3, 0x5);
	encode_bits(sim->width, sim->black, sim->white, sim->guess,
				&sim->sections[45], 5, 0xa);
	encode_bits(sim->width, sim->black, sim->white, sim->guess,
				&sim->sections[92], 3, 0x5);
	// Known sides of digits.
	for (int i = 0; i < 7; i++) {
		encode_bits(sim->width, sim->black, sim->white, sim->guess, &sim->sections[2 + i * 7], 2, 0x2);	// high-low
		encode_bits(sim->width, sim->black, sim->white, sim->guess, &sim->sections[49 + i * 7], 2, 0x1);	// low-high
	}
}


void printable_digits(Simulator *sim, char *output)
{
	for (int i = 0; i < 13; i++) {
		output[i] = (sim->digits[i] % 10) + 48;
	}
	output[13] = 0;
}


void adjust_sigma(Simulator *sim, int steps, double step_size,
				  double factor)
{
	double original_sigma = sim->sigma;
	double best_sigma = sim->sigma;
	double best_error = barcode_error(sim);
	for (int step = -steps; step <= steps; step++) {
		if (step == 0)
			continue;
		sim->sigma = original_sigma + step * step_size;
		if (sim->sigma < 1.0)
			continue;
		compute_kernel(sim);
		convolve_1d(sim->kernel_width, sim->kernel,
					sim->width, sim->guess, sim->blur, 0, sim->width);
		double error = barcode_error(sim);
		// printf("sigma=%.2f error=%.2f\n", sim->sigma, error);
		if (error < best_error) {
			best_error = error;
			best_sigma = sim->sigma;
		}
	}
	sim->sigma = best_sigma * factor + original_sigma * (1.0 - factor);
	compute_kernel(sim);
	convolve_1d(sim->kernel_width, sim->kernel,
				sim->width, sim->guess, sim->blur, 0, sim->width);
}


void simulate_range(Simulator *sim, int start, int stop)
{
	// printf("simulate_range start=%d stop=%d\n", start, stop);
	int x = start;
	int next_section = 0;
	while (next_section < 96 && x > sim->sections[next_section])
		next_section++;
	while (x < stop) {
		// printf("start=%d x=%d stop=%d sections[%d]=%d\n",
		//        start, x, stop, next_section, sim->sections[next_section]);
		if (next_section <= 0) {	// Left shoulder.
			for (; x < sim->sections[0]; x++)
				sim->guess[x] = sim->white[x];
			next_section = 1;
		} else if (next_section <= 3) {	// Left guard bars.
			encode_bits(sim->width, sim->black, sim->white, sim->guess,
						&sim->sections[0], 3, 0x5);
			next_section = 4;
		} else if (next_section <= 45) {	// Left six digits.
			int i = (next_section - 4) / 7;
			encode_bits(sim->width, sim->black, sim->white, sim->guess,
						&sim->sections[3 + 7 * i], 7,
						LG[sim->digits[1 + i]]);
			next_section = 3 + 7 * i + 8;
		} else if (next_section <= 50) {	// Middle guard bars.
			encode_bits(sim->width, sim->black, sim->white, sim->guess,
						&sim->sections[45], 5, 0xa);
			next_section = 51;
		} else if (next_section <= 92) {	// Right six digits.
			int i = (next_section - 51) / 7;
			encode_bits(sim->width, sim->black, sim->white, sim->guess,
						&sim->sections[50 + 7 * i], 7,
						R[sim->digits[7 + i]]);
			next_section = 50 + 7 * i + 8;
		} else if (next_section <= 95) {	// Right guard bars.
			encode_bits(sim->width, sim->black, sim->white, sim->guess,
						&sim->sections[92], 3, 0x5);
			next_section = 96;
		} else {				// Right shoulder.
			for (; x < stop; x++)
				sim->guess[x] = sim->white[x];
			break;
		}
		x = sim->sections[next_section - 1];
	}
	convolve_1d(sim->kernel_width, sim->kernel,
				sim->width, sim->guess, sim->blur, start, stop);
}


void adjust_side(Simulator *sim, int steps, int *side, int start, int stop)
{
	int original_side = *side;
	double best_side = *side;
	double best_error = samples_error(sim, start, stop);
	for (int step = -steps; step <= steps; step++) {
		// if (step == 0) continue;
		*side = original_side + step;
		compute_sections(sim);
		simulate_range(sim, start - EXTRA_SAMPLES, stop + EXTRA_SAMPLES);
		double error = samples_error(sim, start, stop);
		// printf("step=%-3d start=%-4d stop=%-4d error=%.6f\n",
		//        step, sim->start, sim->stop, error);
		if (error < best_error) {
			best_error = error;
			best_side = *side;
		}
	}
	*side = best_side;
	compute_sections(sim);
	simulate_range(sim, start - EXTRA_SAMPLES, stop + EXTRA_SAMPLES);
}


void adjust_start(Simulator *sim, int steps)
{
	// printf("adjust_start...\n");
	adjust_side(sim, steps, &sim->start,
				sim->start - EXTRA_SAMPLES, sim->sections[4]);
	simulate_range(sim,
				   sim->start - EXTRA_SAMPLES, sim->stop + EXTRA_SAMPLES);
}


void adjust_stop(Simulator *sim, int steps)
{
	// printf("adjust_stop...\n");
	adjust_side(sim, steps, &sim->stop,
				sim->sections[91], sim->stop + EXTRA_SAMPLES);
	simulate_range(sim,
				   sim->start - EXTRA_SAMPLES, sim->stop + EXTRA_SAMPLES);
}


void adjust_start_stop(Simulator *sim, int steps)
{
	for (int x = 0; x < sim->width; x++) {
		sim->guess[x] = sim->white[x];
		sim->blur[x] = UNKNOWN;
	}
	adjust_side(sim, steps, &sim->start,
				sim->start - EXTRA_SAMPLES, sim->sections[4]);
	adjust_side(sim, steps, &sim->stop,
				sim->sections[91], sim->stop + EXTRA_SAMPLES);
	simulate_range(sim,
				   sim->start - EXTRA_SAMPLES, sim->stop + EXTRA_SAMPLES);
}
