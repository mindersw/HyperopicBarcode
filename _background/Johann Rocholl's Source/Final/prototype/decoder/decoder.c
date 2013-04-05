#include "decoder.h"
#include "threshold.h"
#include "trace.h"
#include "gradients.h"
#include "roi.h"
#include "quietzones.h"
#include "scanlines.h"
#include "illumination.h"
#include "perspective.h"
#include "simulator.h"
#include "gnuplot.h"
#include "thirty.h"
#include <stdio.h>

#define BLOCK_SIZE 8
#define MAX_BARS 120


void remove_laser(IplImage *red, int top, int height)
{
	for (int y = top; y < top + height; y++) {
		uchar *pixel = (uchar *) (red->imageData + red->widthStep * y);
		for (int x = 0; x < red->width; x++) {
			int p = pixel[0];
			p = (p - 64) * 4 / 3;
			if (p < 0)
				p = 0;
			if (p > 255)
				p = 255;
			pixel[0] = p;
			pixel++;
		}
	}
}


void null_images(AppData *data)
{
	data->red = NULL;
	data->threshold = NULL;
	data->smooth = NULL;
	data->sobel_h = NULL;
	data->sobel_v = NULL;
	data->gradients = NULL;
	data->blocks = NULL;
	data->r = NULL;
	data->g = NULL;
	data->b = NULL;
	data->rgb = NULL;
	data->debug = NULL;
	data->blocks = NULL;
	data->graph = NULL;
}


void create_images(AppData *data, CvSize size, int graph_columns)
{
	data->red = cvCreateImage(size, IPL_DEPTH_8U, 1);
	data->threshold = cvCreateImage(size, IPL_DEPTH_8U, 1);
	// data->smooth = cvCreateImage(size, IPL_DEPTH_8U, 1);
	// data->sobel_h = cvCreateImage(size, IPL_DEPTH_16S, 1);
	// data->sobel_v = cvCreateImage(size, IPL_DEPTH_16S, 1);
	// data->gradients = cvCreateImage(size, IPL_DEPTH_8U, 3);
	data->r = cvCreateImage(size, IPL_DEPTH_8U, 1);
	data->g = cvCreateImage(size, IPL_DEPTH_8U, 1);
	data->b = cvCreateImage(size, IPL_DEPTH_8U, 1);
	data->rgb = cvCreateImage(size, IPL_DEPTH_8U, 3);
	CvSize blocks_size = cvSize(size.width / BLOCK_SIZE,
								size.height / BLOCK_SIZE);
	data->blocks = cvCreateImage(blocks_size, IPL_DEPTH_8U, 3);
	CvSize graph_size = cvSize(graph_columns * size.width, size.height);
	data->graph = cvCreateImage(graph_size, IPL_DEPTH_8U, 3);
	cvInitFont(&data->font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 2, 0);
}


void release_images(AppData *data)
{
	cvReleaseImage(&data->red);
	cvReleaseImage(&data->threshold);
	cvReleaseImage(&data->smooth);
	cvReleaseImage(&data->sobel_h);
	cvReleaseImage(&data->sobel_v);
	cvReleaseImage(&data->gradients);
	cvReleaseImage(&data->r);
	cvReleaseImage(&data->g);
	cvReleaseImage(&data->b);
	cvReleaseImage(&data->rgb);
	cvReleaseImage(&data->blocks);
	cvReleaseImage(&data->graph);
}


void debug_image(AppData *data, IplImage *image)
{
	CvRect roi = cvGetImageROI(data->debug);
	int zoom = 1;
	if (image->width > data->debug->width)
		zoom = image->width / data->debug->width;
	roi.width = image->width / zoom;
	roi.height = image->height / zoom;
	if (roi.x + roi.width > data->debug->width) {
		roi.x = 0;
		roi.y += roi.height;
		if (roi.y + roi.height > data->debug->height)
			roi.y = 0;
	}
	// printf("ROI x=%d y=%d width=%d height=%d\n",
	//        roi.x, roi.y, roi.width, roi.height);
	cvSetImageROI(data->debug, roi);
	cvResize(image, data->debug, CV_INTER_AREA);
	roi.x += roi.width;
	if (roi.x >= data->debug->width) {
		roi.x = 0;
		roi.y += roi.height;
		if (roi.y >= data->debug->height)
			roi.y = 0;
	}
	cvSetImageROI(data->debug, roi);
}


void debug_channels(AppData *data, IplImage *r, IplImage *g, IplImage *b)
{
	if (r)
		cvMerge(r, NULL, NULL, NULL, data->rgb);
	if (g)
		cvMerge(NULL, g, NULL, NULL, data->rgb);
	if (b)
		cvMerge(NULL, NULL, b, NULL, data->rgb);
	debug_image(data, data->rgb);
}


void debug_gradients(AppData *data, IplImage *gradients, int scale)
{
	cvSplit(gradients, data->r, data->g, data->b, NULL);
	cvConvertScale(data->r, data->r, (double) scale, 0.0);
	cvSet(data->g, cvScalarAll(255), NULL);
	cvMerge(data->r, data->g, data->b, NULL, data->rgb);
	cvCvtColor(data->rgb, data->rgb, CV_HSV2RGB);
	debug_image(data, data->rgb);
}


bool decode(AppData *data, double angle_of_view,
			const char *debug, const void *userdata,
			void (*progress_callback) (const void *, float))
{
	if (strstr(debug, "input"))
		debug_channels(data, data->red, data->red, data->red);

	adaptive_threshold(data->red, data->threshold, data->red->width / 60);
	if (strstr(debug, "threshold"))
		debug_channels(data,
					   data->threshold, data->threshold, data->threshold);

	if (strstr(debug, "steps"))
		printf("Tracing barcode lines...\n");
	if (strstr(debug, "trace"))
		cvMerge(data->red, data->red, data->red, NULL, data->rgb);
	CvPoint pt1 = cvPoint(0, data->red->height / 2);
	CvPoint pt2 = cvPoint(data->red->width - 1, data->red->height / 2);
	Bar bars[MAX_BARS];
	int bars_count = trace_barcode(data->red, data->threshold, pt1, pt2,
								   MAX_BARS, bars, data->rgb);
	find_barcode(data->red, data->threshold,
				 bars_count, bars, data->corners, data->rgb);
	if (strstr(debug, "trace"))
		debug_image(data, data->rgb);
	if (progress_callback)
		progress_callback(userdata, 0.1);

	Simulator sim;
	sim.width = MAX_SAMPLES;
	pt1 = cvPoint((data->corners[0].x + data->corners[3].x) / 2,
				  (data->corners[0].y + data->corners[3].y) / 2);
	pt2 = cvPoint((data->corners[1].x + data->corners[2].x) / 2,
				  (data->corners[1].y + data->corners[2].y) / 2);
	sim.distance =
		perspective_distance(angle_of_view, cvGetSize(data->red), pt1,
							 pt2);
	sim.angle =
		perspective_angle(data->corners[0], data->corners[1],
						  data->corners[3], data->corners[2],
						  sim.distance);
	scanlines_median(data->red, 10, 20,
					 data->corners[0], data->corners[1],
					 data->corners[3], data->corners[2],
					 sim.width, sim.input, data->rgb);
	compute_threshold(&sim);
	if (progress_callback)
		progress_callback(userdata, 0.2);
	// printf("samples=%.3f,%.3f,%.3f,%.3f\n",
	//        sim.input[100], sim.input[101], sim.input[102], sim.input[103]);
	if (strstr(debug, "median"))
		debug_image(data, data->rgb);

	if (strstr(debug, "steps"))
		printf("Finding left and right side of barcode...\n");
	cvSet(data->graph, cvScalarAll(255), NULL);
	cvLine(data->graph, cvPoint(0, 19 * data->graph->height / 40),
		   cvPoint(data->graph->width, 19 * data->graph->height / 40),
		   cvScalar(0, 0, 0, 255), 1, 8, 0);
	sim.start = 5 * sim.width / 100;
	sim.stop = 95 * sim.width / 100;
	estimate_start_stop(sim.width, sim.input, &sim.start, &sim.stop);
	thirty_peaks(&sim);
	printf("start=%d stop=%d count=%d\n",
		   sim.start, sim.stop, sim.stop - sim.start);
	if (progress_callback)
		progress_callback(userdata, 0.3);
	if (strstr(debug, "quietzones"))
		debug_image(data, data->graph);

	if (strstr(debug, "steps"))
		printf("Initial estimates...\n");
	if (sim.stop - sim.start < MIN_SAMPLES)
		return false;
	if (sim.start - 20 < 0 || sim.stop + 20 >= sim.width)
		return false;
	float white_left = sim.input[sim.start - 20];
	float white_right = sim.input[sim.stop + 20];
	fill_scanline(sim.width, sim.white, 0.0, 0, sim.width);
	add_ramp(sim.width, sim.white, sim.start, sim.stop,
			 white_left, white_right);
	float dark;
	median(sim.width, sim.input, sim.start, sim.stop,
		   NULL, NULL, NULL, &dark, NULL);
	fill_scanline(sim.width, sim.black, dark, 0, sim.width);
	if (strstr(debug, "sampling")) {
		scanlines_median(data->red, 1, 20,
						 data->corners[0], data->corners[1],
						 data->corners[3], data->corners[2],
						 sim.width, sim.input, data->rgb);
		scanlines_median(data->red, 40, 20,
						 data->corners[0], data->corners[1],
						 data->corners[3], data->corners[2],
						 sim.width, sim.guess, data->rgb);
		sim.stop = 200;
		plot_simulator(&sim, "input guess key", "gnuplot/robust_sampling");
		return false;
	}

	sim.sigma = MAX_SAMPLES / 150.0;
	compute_kernel(&sim);
	compute_sections(&sim);
	initial_guess(&sim);
	if (strstr(debug, "gnuplot"))
		plot_simulator(&sim, "sections guess", "gnuplot/known_bits");
	if (progress_callback)
		progress_callback(userdata, 0.4);

	if (strstr(debug, "steps"))
		printf("Guessing digits...\n");
	guess_digits(&sim);
	if (progress_callback)
		progress_callback(userdata, 0.5);
	correct_errors(&sim, 3, 20);
	if (progress_callback)
		progress_callback(userdata, 0.6);
	if (strstr(debug, "simulator1")) {
		draw_simulator(&sim, data->graph, &data->font);
		debug_image(data, data->graph);
	}

	if (strstr(debug, "steps"))
		printf("Adjusting parameters and guessing again...\n");
	adjust_start_stop(&sim, 8);
	adjust_sigma(&sim, 10, 1.0, 0.5);
	adjust_light_levels(&sim, 0.5);
	if (progress_callback)
		progress_callback(userdata, 0.7);
	guess_digits(&sim);
	if (progress_callback)
		progress_callback(userdata, 0.75);
	correct_errors(&sim, 3, 20);
	if (progress_callback)
		progress_callback(userdata, 0.8);
	if (strstr(debug, "simulator2")) {
		draw_simulator(&sim, data->graph, &data->font);
		debug_image(data, data->graph);
	}

	if (strstr(debug, "steps"))
		printf("Guessing digits...\n");
	adjust_start_stop(&sim, 8);
	adjust_sigma(&sim, 10, 0.5, 1.0);
	adjust_light_levels(&sim, 1.0);
	if (progress_callback)
		progress_callback(userdata, 0.9);
	guess_digits(&sim);
	if (progress_callback)
		progress_callback(userdata, 0.95);
	if (strstr(debug, "gnuplot"))
		plot_simulator(&sim, "sections input simulated key",
					   "gnuplot/before_error_correction");
	correct_errors(&sim, 3, 20);
	if (strstr(debug, "gnuplot"))
		plot_simulator(&sim, "sections input simulated key",
					   "gnuplot/after_error_correction");
	if (progress_callback)
		progress_callback(userdata, 1.0);
	if (strstr(debug, "simulator3")) {
		draw_simulator(&sim, data->graph, &data->font);
		debug_image(data, data->graph);
	}

	printable_digits(&sim, data->digits);
	return sim.error < 0.7;
}
