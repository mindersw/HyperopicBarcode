#include "decoder.h"
#include "gradients.h"
#include <stdio.h>

#define BLOCK_SIZE 8

// #define DEBUG_SMOOTH
// #define DEBUG_SOBEL
#define DEBUG_GRADIENTS
#define DEBUG_BLOCKS


void copy_channel(const IplImage* input, IplImage* output, IplImage* channel,
					int input_channel, int output_channel, int scale)
{
	switch (input_channel) {
		case 1: cvSplit(input, channel, NULL, NULL, NULL); break;
		case 2:	cvSplit(input, NULL, channel, NULL, NULL); break;
		case 3:	cvSplit(input, NULL, NULL, channel, NULL); break;
		case 4:	cvSplit(input, NULL, NULL, NULL, channel); break;
	}
	if (scale > 1) cvConvertScale(channel, channel, (double) scale, 0.0);
	switch (output_channel) {
		case 1: cvMerge(channel, NULL, NULL, NULL, output); break;
		case 2: cvMerge(NULL, channel, NULL, NULL, output); break;
		case 3: cvMerge(NULL, NULL, channel, NULL, output); break;
		case 4: cvMerge(NULL, NULL, NULL, channel, output); break;
	}
}


void debug_gradients(const IplImage* gradients, int scale, IplImage* channel, IplImage* debug)
{
	CvSize size = cvGetSize(gradients);
	IplImage* hsv = cvCreateImage(size, IPL_DEPTH_8U, 3);
	copy_channel(gradients, hsv, channel, 1, 1, scale); // Hue.
	cvSet(channel, cvScalarAll(255), NULL);
	cvMerge(NULL, channel, NULL, NULL, hsv); // Saturation.
	copy_channel(gradients, hsv, channel, 3, 3, 1); // Value.
	cvCvtColor(hsv, debug, CV_HSV2RGB);
	cvSet(channel, cvScalarAll(255), NULL);
	cvMerge(NULL, NULL, NULL, channel, debug); // Alpha.
	cvReleaseImage(&hsv);
}


bool decode(const IplImage* red, char* digits, IplImage* debug)
{
	CvSize size = cvGetSize(red);
	IplImage* channel = cvCreateImage(size, IPL_DEPTH_8U, 1);
	IplImage* smooth = NULL;
	IplImage* sobel_h = NULL;
	IplImage* sobel_v = NULL;
	IplImage* gradients = NULL;
	IplImage* blocks = NULL;
	
	// Gaussian smoothing of red channel.
	smooth = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cvSmooth(red, smooth, CV_GAUSSIAN, 5, 5, 0, 0);
#ifdef DEBUG_SMOOTH
	cvMerge(smooth, NULL, NULL, NULL, debug);
	cvMerge(NULL, smooth, NULL, NULL, debug);
	cvMerge(NULL, NULL, smooth, NULL, debug);
	goto done;
#endif

	// Horizontal and vertical Sobel.
	sobel_h = cvCreateImage(size, IPL_DEPTH_16S, 1);
	sobel_v = cvCreateImage(size, IPL_DEPTH_16S, 1);
	cvSobel(smooth, sobel_h, 1, 0, CV_SCHARR);
	cvSobel(smooth, sobel_v, 0, 1, CV_SCHARR);
	cvReleaseImage(&smooth);
#ifdef DEBUG_SOBEL
	cvConvertScaleAbs(sobel_h, channel, 1.0, 0.0);
	cvMerge(channel, NULL, NULL, NULL, debug);
	cvConvertScaleAbs(sobel_v, channel, 1.0, 0.0);
	cvMerge(NULL, channel, NULL, NULL, debug);
	cvSetZero(channel);
	cvMerge(NULL, NULL, channel, NULL, debug);
	goto done;
#endif
	
	// Find gradients.
	gradients = cvCreateImage(size, IPL_DEPTH_8U, 3);
	find_gradients(sobel_h, sobel_v, gradients);
	cvReleaseImage(&sobel_v);
	cvReleaseImage(&sobel_h);
#ifdef DEBUG_GRADIENTS
	debug_gradients(gradients, 11, channel, debug);
	goto done;
#endif
	
	// Voting for common directions.
	CvSize blocks_size = cvSize(size.width / BLOCK_SIZE,
								size.height / BLOCK_SIZE);
	printf("blocks: width=%d height=%d\n",
		   blocks_size.width, blocks_size.height);
	blocks = cvCreateImage(blocks_size, IPL_DEPTH_8U, 3);
	vote_blocks(gradients, blocks);
#ifdef DEBUG_BLOCKS
	cvSmooth(blocks, blocks, CV_GAUSSIAN, 5, 5, 0, 0);
	cvResize(blocks, gradients, CV_INTER_NN);
	debug_gradients(gradients, 22, channel, debug);
	goto done;
#endif
	
done:
	cvReleaseImage(&blocks);
	cvReleaseImage(&gradients);
	cvReleaseImage(&sobel_v);
	cvReleaseImage(&sobel_h);
	cvReleaseImage(&smooth);
	cvReleaseImage(&channel);
	return false;
}
