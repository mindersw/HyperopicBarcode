//
//  FullscreenPickerController.h
//  BlurCode
//
//  Created by Johann C. Rocholl on 2/19/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <cv.h>

#define HAVE_BOOL
#import "decoder.h"

@interface FullscreenPickerController : UIImagePickerController {
	uchar *pixels;
	CGContextRef context;
	IplImage *rgba;
	AppData *data;
	NSTimer *captureTimer;
	UIProgressView *progress;
	UIImageView *display;
	NSOperationQueue *queue;
	bool decoding;
}

@end
