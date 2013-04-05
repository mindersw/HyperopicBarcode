//
//  FullscreenPickerController.m
//  BlurCode
//
//  Created by Johann C. Rocholl on 2/19/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "FullscreenPickerController.h"
#import "DecoderOperation.h"
#import <QuartzCore/QuartzCore.h>
#import <cv.h>

#define SCREENSHOT_WIDTH 320
#define SCREENSHOT_HEIGHT 240
#define LASER_TOP 118
#define LASER_HEIGHT 4
#define PROGRESS_MARGIN 40
#define PROGRESS_Y 230

extern CGImageRef UIGetScreenImage();

@implementation FullscreenPickerController


- (void)viewDidLoad {
    [super viewDidLoad];

#if TARGET_IPHONE_SIMULATOR
	// Fake camera image.
	CGRect previewFrame = CGRectMake(0, 0, 320, 480);
    UIImageView* preview = [[UIImageView alloc] initWithFrame:previewFrame];
	preview.image = [UIImage imageNamed:@"Default.png"];
    [self.view addSubview:preview];
    [preview release];
#endif

	// Laser line.
	CGRect redFrame = CGRectMake(0, LASER_TOP, 320, LASER_HEIGHT);
	UIView* red = [[UIView alloc] initWithFrame:redFrame];
	[red setBackgroundColor:[UIColor redColor]];
	[red setAlpha:0.25];
	[self.view addSubview:red];
	[red release];

	// Progress bar.
	CGRect progressFrame = CGRectMake(PROGRESS_MARGIN, PROGRESS_Y - 5, 320 - 2 * PROGRESS_MARGIN, 10);
	progress = [[UIProgressView alloc] initWithFrame:progressFrame];
	progress.progressViewStyle = UIProgressViewStyleBar;
	progress.hidden = YES;
    [self.view addSubview:progress];
	
	// Result output.
	CGRect displayFrame = CGRectMake(0, SCREENSHOT_HEIGHT, 320, 480);
    display = [[UIImageView alloc] initWithFrame:displayFrame];
	[display setOpaque:YES];
    [self.view addSubview:display];
	
	// Operation queue for background worker thread.
	queue = [[NSOperationQueue alloc] init];

	// Application data.
	data = malloc(sizeof(AppData));
	
	// Pixel buffer for screenshots.
    pixels = malloc(320 * 480 * 4);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    context = CGBitmapContextCreate(
		pixels, 320, 480, 8, 320 * 4, colorspace,
		kCGImageAlphaPremultipliedLast);
	// Create image header over pixel data for use with OpenCV.
	CvSize size = cvSize(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT);
	rgba = cvCreateImageHeader(size, IPL_DEPTH_8U, 4);
	cvSetData(rgba, pixels, 320 * 4);
}


- (void)viewDidAppear: (BOOL)animated {
    [super viewDidAppear:animated];
	// Hide standard controls
    UIView* crop = [[[[[[[[self.view subviews] objectAtIndex:0]
						 subviews] objectAtIndex:0]
					   subviews] objectAtIndex:0]
					 subviews] objectAtIndex:3];
	[crop setHidden:YES];

	bool responds = [self respondsToSelector:@selector(updateProgress:)];
	NSLog(@"responds=%d", responds);
	if (!responds) return;
	
	// Allocate images for processing.
	create_images(data, cvSize(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT), 2);
	data->debug = cvCreateImage(cvSize(SCREENSHOT_WIDTH, SCREENSHOT_HEIGHT), IPL_DEPTH_8U, 3);
	
	// Create a timer for screen captures.
	decoding = false;
    captureTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
					target:self selector:@selector(capture)
					userInfo:nil repeats:YES];
}


- (void)viewDidDisappear:(BOOL)animated {
    captureTimer = nil;
	release_images(data);
	cvReleaseImage(&data->debug);
    [super viewDidDisappear:animated];
}


- (void)capture {
	if (decoding) return;
	decoding = true;
	
	// Extract RGBA pixel data from screenshot.
	CGImageRef imgRef = UIGetScreenImage();	
    size_t width = CGImageGetWidth(imgRef);
    size_t height = CGImageGetHeight(imgRef);
    CGRect rect = CGRectMake(0, 0, width, height);
    CGContextDrawImage(context, rect, imgRef);
	CFRelease(imgRef);	

	// Extract red color channel.
	cvSplit(rgba, data->red, nil, nil, nil);
	remove_laser(data->red, LASER_TOP, LASER_HEIGHT);
	
	[progress setProgress:0.0];
	[progress setHidden:NO];

	DecoderOperation *operation = [[DecoderOperation alloc] init];
	operation.data = data;
	operation.delegate = self;
	[queue addOperation:operation];
	[operation release];
}


- (void)updateProgress: (id)object {
	NSNumber *number = (NSNumber*) object;
	float value = [number floatValue];
	progress.progress = value;
}


- (void)decoderFinished: (id)object {
	progress.hidden = YES;
 
	cvResetImageROI(data->debug);
	cvSplit(data->debug, data->r, NULL, NULL, NULL);
	cvSplit(data->debug, NULL, data->g, NULL, NULL);
	cvSplit(data->debug, NULL, NULL, data->b, NULL);
	cvSet(data->red, cvScalarAll(255), NULL);
	cvMerge(data->r, data->g, data->b, data->red, rgba);
	CGImageRef outputRef = CGBitmapContextCreateImage(context);
	UIImage* screen = [UIImage imageWithCGImage:outputRef];
	display.image = screen;
	// UIImageWriteToSavedPhotosAlbum(screen, nil, nil, nil);

	CFRelease(outputRef);
	decoding = false;
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
