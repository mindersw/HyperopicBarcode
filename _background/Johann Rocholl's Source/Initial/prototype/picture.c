#include <stdio.h>
#include <unistd.h>
#include <cv.h>
#include <highgui.h>

#include "minmax.h"
#include "boolean.h"
#include "decoder.h"

// For the test suite.
#include "dir8.h"
#include "dir16.h"
#include "checksums.h"

#define ANGLE_OF_VIEW 46.0 / 180.0 * CV_PI
#define COLUMNS 2
#define ROWS 2


int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: picture [options] <infile> ...\n");
		fprintf(stderr, "options:  --test    run test suite\n");
		fprintf(stderr,
				"          --force   don't skip existing output files\n");
		return 1;
	}
	char basename[100];
	char outfilename[200];
	char linkfilename[200];
	bool force = false;
	AppData data;
	null_images(&data);
	for (int n = 1; n < argc; n++) {
		if (strcmp(argv[n], "--test") == 0) {
			printf("Running dir8 test suite...\n");
			dir8_test();
			printf("Running dir16 test suite...\n");
			dir16_test();
			printf("Running checksums test suite...\n");
			checksums_test();
			continue;
		} else if (strcmp(argv[n], "--force") == 0) {
			force = true;
			continue;
		}
		char *infilename = argv[n];
		char *slash = strrchr(infilename, '/');
		if (!slash)
			slash = infilename - 1;
		strcpy(basename, slash + 1);
		char *ext = strrchr(basename, '.');
		if (ext)
			ext[0] = 0;
		strcpy(outfilename, "output/");
		strcat(outfilename, basename);
		strcat(outfilename, ".png");
		strcpy(linkfilename, "digits/");
		strcat(linkfilename, basename);
		if (!force) {
			FILE *fp = fopen(outfilename, "r");
			if (fp) {			// Skip existing output file.
				fclose(fp);
				continue;
			}
		}
		printf("%s => %s\n", infilename, outfilename);
		IplImage *input = cvLoadImage(infilename, CV_LOAD_IMAGE_COLOR);
		if (!input) {
			fprintf(stderr, "could not read %s\n", infilename);
			return 1;
		}
		int zoom = max(1, max(input->width, input->height) / 600);
		if (data.red == NULL ||
			data.red->width != input->width / zoom ||
			data.red->height != input->height / zoom) {
			if (data.red)
				release_images(&data);
			if (data.debug)
				cvReleaseImage(&data.debug);
			CvSize zoom_size =
				cvSize(input->width / zoom, input->height / zoom);
			create_images(&data, zoom_size, 4);
			CvSize debug_size = cvSize(COLUMNS * input->width / zoom,
									   ROWS * input->height / zoom);
			data.debug = cvCreateImage(debug_size, IPL_DEPTH_8U, 3);
			printf("input=%dx%d zoom=%dx%d debug=%dx%d\n",
				   input->width, input->height,
				   data.red->width, data.red->height,
				   data.debug->width, data.debug->height);
		}
		// printf("Resizing input from %dx%d to %dx%d...\n",
		// input->width, input->height, data.rgb->width, data.rgb->height);
		cvCvtColor(input, input, CV_BGR2RGB);
		cvResize(input, data.rgb, CV_INTER_AREA);
		cvSplit(data.rgb, data.red, NULL, NULL, NULL);
		// gettimeofday(&data.started, NULL);
		// debug_image(&data, data.rgb);
		bool success = decode(&data, ANGLE_OF_VIEW,
							  "steps bars frame simulator1 simulator3",
							  NULL, NULL);
		if (success) {
			unlink(linkfilename);
			symlink(data.digits, linkfilename);
			printf("digits=%s\n", data.digits);
		}
		cvResetImageROI(data.debug);
		cvCvtColor(data.debug, data.debug, CV_RGB2BGR);
		if (cvSaveImage(outfilename, data.debug) == 0) {
			fprintf(stderr, "could not save output file\n");
			return 1;
		}
	}
	if (data.red)
		release_images(&data);
	if (data.debug)
		cvReleaseImage(&data.debug);
	return 0;
}
