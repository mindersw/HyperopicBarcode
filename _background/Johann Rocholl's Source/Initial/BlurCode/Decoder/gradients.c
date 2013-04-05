#include "gradients.h"
#include "dir8.h"
#include "dir16.h"
#include "minmax.h"
#include <stdio.h>


// All input and output images must have same dimensions.
// ROI on output image is respected.
void find_gradients(const IplImage* sobel_h, const IplImage* sobel_v,
					IplImage* gradients)
{
  CvRect roi = cvGetImageROI(gradients);
  for (int y = roi.y; y < roi.y + roi.height; y++) {
	uchar* ptr_g = (uchar*) (gradients->imageData + gradients->widthStep * y);
	short* ptr_h = (short*) (sobel_h->imageData + sobel_h->widthStep * y);
	short* ptr_v = (short*) (sobel_v->imageData + sobel_v->widthStep * y);
	for (int x = roi.x; x < roi.x + roi.width; x++) {
	  uchar* pixel = ptr_g + gradients->nChannels * x;
	  int v = ptr_v[x];
	  int h = ptr_h[x];
	  pixel[0] = dir16_atan2(v, h);
	  pixel[1] = dir8_atan2(v, h);
	  int s = (int) sqrt(v * v + h * h) / 4;
	  pixel[2] = s > 255 ? 255 : s;
	}
  }
}


// Output must be allocated with correct size before call.
// ROI is ignored.
void vote_blocks(const IplImage* gradients, IplImage* blocks)
{
  int BLOCK_SIZE = gradients->width / blocks->width;
  for (int by = 0; by < blocks->height; by++) {
	uchar* blocks_ptr =
	  (uchar*) (blocks->imageData + blocks->widthStep * by);
	int y_stop = (by + 1) * BLOCK_SIZE;
	if (y_stop > gradients->height) y_stop = gradients->height;
    for (int bx = 0; bx < blocks->width; bx++) {
      int x_stop = (bx + 1) * BLOCK_SIZE;
      if (x_stop > gradients->width) x_stop = gradients->width;
      int votes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      int intensity[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      for (int y = by * BLOCK_SIZE; y < y_stop; y++) {
		uchar* gradients_ptr =
		  (uchar*) (gradients->imageData + gradients->widthStep * y);
		for (int x = bx * BLOCK_SIZE; x < x_stop; x++) {
		  uchar* pixel = gradients_ptr + gradients->nChannels * x;
		  int d = pixel[0];
		  votes[d % 8] += 2;
		  votes[(d + 1) % 8]++;
		  votes[(d + 7) % 8]++;
		  if (intensity[d % 8] < pixel[2])
			  intensity[d % 8] = pixel[2];
		}
      }
	  int best_dir16 = 0;
      int most_votes = 0;
      for (int d = 0; d < 8; d++) {
		if (votes[d] > most_votes) {
		  most_votes = votes[d];
		  best_dir16 = d;
		}
      }
	  // printf("bx=%d by=%d most_votes=%d best_dir16=%d\n",
	  //  		 bx, by, most_votes, best_dir16);
	  uchar* pixel = blocks_ptr + blocks->nChannels * bx;
	  pixel[0] = best_dir16;
	  pixel[1] = most_votes > 255 ? 255 : most_votes;
	  pixel[2] = intensity[best_dir16];
    }
  }
}


// Input and output must be the same size.
// Output should be a copy of input before call.
// ROI on output is respected.
// ROI must be 2 pixels away from each border, or it will be adjusted.
// Non-maxima pixels will have their gradient intensity set to zero.
void suppress_non_maxima(const IplImage* gradients, IplImage* maxima)
{
  CvRect roi = cvGetImageROI(maxima);
  int x_start = max(roi.x, 2);
  int y_start = max(roi.y, 2);
  int x_stop = min(roi.x + roi.width, gradients->width - 2);
  int y_stop = min(roi.y + roi.height, gradients->height - 2);
  int gstep = gradients->widthStep;
  for (int y = y_start; y < y_stop; y++) {
	uchar* gline = (uchar*) (gradients->imageData + gradients->widthStep * y);
	uchar* mline = (uchar*) (maxima->imageData + maxima->widthStep * y);
    for (int x = x_start; x < x_stop; x++) {
	  uchar* gpixel = gline + 3 * x;
	  uchar* mpixel = mline + 3 * x;
	  uchar gvalue = gpixel[2];
      switch (gpixel[1]) {
      case 0: // Right
      case 4: // Left
		if (gvalue < gpixel[-1] ||
			gvalue < gpixel[+5])
		  mpixel[2] = 0;
		break;
      case 1: // Down right
      case 5: // Up left
		if (gvalue < gpixel[-1 - 1 * gstep] ||
			gvalue < gpixel[+5 + 1 * gstep])
		  mpixel[2] = 0;
		break;
      case 2: // Down
      case 6: // Up
		if (gvalue < gpixel[2 - 1 * gstep] ||
		    gvalue < gpixel[2 + 1 * gstep])
		  mpixel[2] = 0;
		break;
      case 3: // Down left
      case 7: // Up right
		if (gvalue < gpixel[-1 + 1 * gstep] ||
		    gvalue < gpixel[+5 - 1 * gstep])
		  mpixel[2] = 0;
		break;
      }
    }
  }
}
