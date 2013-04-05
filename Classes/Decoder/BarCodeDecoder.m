//	(c) Copyright 2012 Conor Dearden & Michael LaMorte
//  Portions (c) Copyright 2008-2009 Johann C. Rocholl
//
//
//  BarCodeDecoder.m
//  
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

#import "BarCodeDecoder.h"
#import "gradients.h"
#import "imgproc_c.h"
#import "types_c.h"
#import "cv.h"
#include "decoder.h"

#define ANGLE_OF_VIEW 46.0 / 180.0 * CV_PI
#define COLUMNS 2
#define ROWS 1

//#define BLOCK_SIZE 8


@implementation BarCodeDecoder

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
 
-(NSString *)decode:(IplImage*)input {
		
	AppData data = {0};
    
	// Begin Johann's code from camera.c
	CvSize size = cvSize(input->width, input->height); // Original code halved both of these
	create_images(&data, size, 3);
	CvSize debug_size = cvSize(COLUMNS * size.width, ROWS * size.height);
	data.debug = cvCreateImage(debug_size, IPL_DEPTH_8U, 3);
	cvCvtColor(input, input, CV_BGR2RGB);
	cvResize(input, data.rgb, CV_INTER_AREA);
	debug_image(&data, data.rgb);
	cvSplit(data.rgb, data.red, NULL, NULL, NULL);
	cvSmooth(data.red, data.red, CV_GAUSSIAN, 3, 3, 0, 0);
	
	BOOL success = decode(&data, ANGLE_OF_VIEW, "trace bars gnuplot simulator3", NULL, NULL); //steps
	
		
	// create NSString from char array
	NSMutableString *returnValue = [[NSMutableString alloc] initWithCapacity:14];
	int i;
	for (i = 0; i < 14; i++) {
		[returnValue appendFormat:@"%c", data.digits[i]];
	}
    
	cvResetImageROI(data.debug);
	cvCvtColor(data.debug, data.debug, CV_RGB2BGR);
	cvReleaseImage(&data.debug);
	release_images(&data);
	
	if (success) {
		NSString *returnString = [NSString stringWithString:(NSString *)returnValue];
		[returnValue release];
		return returnString;
	}
	
	[returnValue release];
	return [NSString string];
}


+(NSString *)decode:(IplImage*)input {
    
	AppData data = {0};
    
	// Begin Johann's code from camera.c
	CvSize size = cvSize(input->width / 2, input->height / 2);
	create_images(&data, size, 3);
	CvSize debug_size = cvSize(COLUMNS * size.width, ROWS * size.height);
	data.debug = cvCreateImage(debug_size, IPL_DEPTH_8U, 3);
	//cvCvtColor(input, input, CV_BGR2RGB);
	//cvResize(input, data.rgb, CV_INTER_AREA);
	//debug_image(&data, data.rgb);
	//cvSplit(data.rgb, data.red, NULL, NULL, NULL);
	//cvSmooth(data.red, data.red, CV_GAUSSIAN, 3, 3, 0, 0);
	// ^^^ from original
    
	// \/ \/ \/ from final:
	cvCvtColor(input, input, CV_BGR2RGB);
	cvResize(input, data.rgb, CV_INTER_AREA);
	cvSplit(data.rgb, data.red, NULL, NULL, NULL);
	cvSmooth(data.red, data.red, CV_GAUSSIAN, 3, 3, 0, 0);
	//decode(&data, ANGLE_OF_VIEW, "input trace", NULL, NULL);
	BOOL success = decode(&data, ANGLE_OF_VIEW, "trace bars gnuplot simulator3", NULL, NULL); //steps
	cvResetImageROI(data.debug);
	cvCvtColor(data.debug, data.debug, CV_RGB2BGR);
    
	// create NSString from char array
	NSMutableString *returnValue = [[NSMutableString alloc] initWithCapacity:14];
	int i;
	for (i = 0; i < 14; i++) {
		[returnValue appendFormat:@"%c", data.digits[i]];
	}
    
	cvResetImageROI(data.debug);
	cvCvtColor(data.debug, data.debug, CV_RGB2BGR);
	cvReleaseImage(&data.debug);
	release_images(&data);
	
	if (success) {
		NSString *returnString = [NSString stringWithString:(NSString *)returnValue];
		[returnValue release];
		return returnString;
	}
	
	[returnValue release];
	return [NSString string];

}


@end
