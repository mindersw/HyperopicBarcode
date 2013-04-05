//
//  DecoderOperation.m
//  BlurCode
//
//  Created by Johann C. Rocholl on 3/1/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "DecoderOperation.h"

#define ANGLE_OF_VIEW 46.0 / 180.0 * CV_PI


void progress_callback(const void* userdata, float value)
{
	NSNumber *number = [NSNumber numberWithFloat:value];
	UIViewController *delegate = (UIViewController*) userdata;
	[delegate performSelectorOnMainThread:@selector(updateProgress:) withObject:number waitUntilDone:NO];
}


@implementation DecoderOperation

@synthesize data;
@synthesize delegate;

- (void)main {
	NSLog(@"main: size=%dx%d", data->red->width, data->red->height);
	bool success = decode(data, ANGLE_OF_VIEW, "steps simulator2 simulator3",
						  (const void*) delegate, &progress_callback);
	if (success) NSLog(@"digits=%s", data->digits);
	[delegate performSelectorOnMainThread:@selector(decoderFinished:) withObject:self waitUntilDone:NO];
}

@end
