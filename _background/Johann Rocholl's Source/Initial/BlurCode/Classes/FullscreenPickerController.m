//
//  FullscreenPickerController.m
//  BlurCode
//
//  Created by Johann C. Rocholl on 2/19/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "FullscreenPickerController.h"
#import <QuartzCore/QuartzCore.h>
#import <cv.h>

#define HAVE_BOOL
#import "decoder.h"

extern CGImageRef UIGetScreenImage();

@implementation FullscreenPickerController

@synthesize captureTimer;


- (void)viewDidLoad {
    [super viewDidLoad];
#if TARGET_IPHONE_SIMULATOR
	CGRect previewFrame = CGRectMake(0, 0, 320, 480);
    UIImageView *preview = [[UIImageView alloc] initWithFrame:previewFrame];
	preview.image = [UIImage imageNamed:@"Default.png"];
    [self.view addSubview:preview];
    [preview release];
#endif
	CGRect displayFrame = CGRectMake(0, 240, 320, 480);
    UIImageView *display = [[UIImageView alloc] initWithFrame:displayFrame];
    [self.view addSubview:display];
    [display release];
}


-(void) viewDidAppear: (BOOL)animated {
    [super viewDidAppear:animated];
	// Hide standard controls
    UIView *crop = [[[[[[[[self.view subviews] objectAtIndex:0]
						 subviews] objectAtIndex:0]
					   subviews] objectAtIndex:0]
					 subviews] objectAtIndex:3];
	[crop setHidden:YES];
    self.captureTimer = [NSTimer scheduledTimerWithTimeInterval:3.0 // Three seconds.
                                 target:self selector:@selector(capture)
								 userInfo:nil repeats:YES];	
}


- (void)viewDidDisappear:(BOOL)animated {
    self.captureTimer = nil;
    [super viewDidDisappear:animated];
}


-(void) capture {
	CGImageRef imgRef = UIGetScreenImage();	
    size_t width = CGImageGetWidth(imgRef);
    size_t height = CGImageGetHeight(imgRef) / 2;
    NSLog(@"width=%d height=%d", width, height);
	
	// Extract RGBA pixel data from CGImageRef.
    unsigned char* pixels = malloc(width * height * 4);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGContextRef contextRef = CGBitmapContextCreate(pixels, width, height, 8, width * 4, colorspace, kCGImageAlphaPremultipliedLast);
    CGRect rect = CGRectMake(0, 0, width, height);
    CGContextDrawImage(contextRef, rect, imgRef);

	// Create matching image header for OpenCV.
	CvSize size = cvSize(width, height);
	IplImage* rgba = cvCreateImageHeader(size, IPL_DEPTH_8U, 4);
	cvSetData(rgba, pixels, width * 4);
	
	// Extract red color channel.
	IplImage* red = cvCreateImage(size, IPL_DEPTH_8U, 1);
	cvSplit(rgba, red, nil, nil, nil);
	
	// Attempt to decode barcode.
	char digits[14];
	bool success = decode(red, digits, rgba);
	if (success) NSLog(@"digits=%s", digits);
	
	CGImageRef outputRef = CGBitmapContextCreateImage(contextRef);
	UIImage* screen = [UIImage imageWithCGImage:outputRef];
	UIImageView* display = [[self.view subviews] lastObject];
	display.image = screen;
	// UIImageWriteToSavedPhotosAlbum(screen, nil, nil, nil);

	CFRelease(outputRef);
	cvReleaseImage(&red);
	cvReleaseImageHeader(&rgba);
	CGContextRelease(contextRef);
    CGColorSpaceRelease(colorspace);
    free(pixels);
	CFRelease(imgRef);
}

	
- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}


- (void)dealloc {
	[captureTimer release];
    [super dealloc];
}


@end
