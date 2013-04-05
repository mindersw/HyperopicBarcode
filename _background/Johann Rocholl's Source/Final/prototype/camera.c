#include <stdio.h>
#include <cv.h>
#include <highgui.h>

#include "decoder.h"

#define ANGLE_OF_VIEW 46.0 / 180.0 * CV_PI
#define COLUMNS 2
#define ROWS 1


CvCapture *init_camera()
{
	CvCapture *capture = cvCreateCameraCapture(CV_CAP_ANY);
	if (!capture) {
		fprintf(stderr, "ERROR: capture is NULL\n");
		exit(1);
	}
	return capture;
}


void init_window(const char *name, int left, int top)
{
	cvNamedWindow(name, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(name, left, top);
}


IplImage *query_frame(CvCapture *capture)
{
	IplImage *frame = cvQueryFrame(capture);
	if (!frame) {
		fprintf(stderr, "ERROR: frame is NULL\n");
		exit(1);;
	}
	return frame;
}


int main()
{
	init_window("Blurcode", 320, 80);
	CvCapture *capture = init_camera();
	IplImage *input = query_frame(capture);
	AppData data;
	CvSize size = cvSize(input->width / 2, input->height / 2);
	create_images(&data, size, 3);
	CvSize debug_size = cvSize(COLUMNS * size.width, ROWS * size.height);
	data.debug = cvCreateImage(debug_size, IPL_DEPTH_8U, 3);
	printf("camera=%dx%d zoomed=%dx%d debug=%dx%d\n",
		   input->width, input->height, size.width, size.height,
		   data.debug->width, data.debug->height);
	int delay = 100;
	while ((cvWaitKey(delay) & 255) != 27) {
		delay = 10;
		input = query_frame(capture);
		cvCvtColor(input, input, CV_BGR2RGB);
		// Horizontal flip (mirror display)
		cvFlip(input, input, 1);
		cvResize(input, data.rgb, CV_INTER_AREA);
		cvSplit(data.rgb, data.red, NULL, NULL, NULL);
		cvSmooth(data.red, data.red, CV_GAUSSIAN, 3, 3, 0, 0);
		decode(&data, ANGLE_OF_VIEW, "input trace", NULL, NULL);
		cvResetImageROI(data.debug);
		cvCvtColor(data.debug, data.debug, CV_RGB2BGR);
		cvShowImage("Blurcode", data.debug);
	}
	cvReleaseCapture(&capture);
	cvReleaseImage(&data.debug);
	release_images(&data);
	cvDestroyWindow("Blurcode");
	return 0;
}
