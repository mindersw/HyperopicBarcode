//	(c) Copyright 2012-2014 Conor Dearden & Michael LaMorte
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

#include "core_c.h"
#include "imgproc_c.h"

#import "BarcodeController.h"
#import <QTKit/QTKit.h>
#import <QuickTime/QuickTime.h>
#import <QuartzCore/CIImage.h>
#import <QuartzCore/QuartzCore.h>
#import "BarCodeDecoder.h"
#import <CommonCrypto/CommonDigest.h>

#import "XMLReader.h"

#define ACCESS_KEY @"YOUR_ACCESS_KEY_HERE"
#define SECRET_KEY @"YOUR_SECRET_KEY_HERE"
#define ASSOCIATE_TAG @"YOUR_ASSOCIATE_TAG_HERE"

@implementation BarcodeController

@synthesize last1, last2, currentLocale;

- (void)awakeFromNib {
	
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(handleScannerWindowWillClose:)
												 name:NSWindowWillCloseNotification
											   object:barcodeScannerWindow];
    [[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(handleScannerWindowWillOpen:)
												 name:NSWindowDidBecomeMainNotification
											   object:barcodeScannerWindow];
	
	[qtCaptureViewer setDelegate:self];
	[qtCaptureViewer setWantsLayer:YES];
	[barcodeLines setWantsLayer:YES];
	[cameraNameLabel setWantsLayer:YES];
	[detailLabel setWantsLayer:YES];
	[detailLabel setStringValue:@""];
	
	
	keepScanning = 1;
	resetFrames = 0;
	frameCount = 0;
	
	// set default lookup locale
	NSLocale *locale = [NSLocale currentLocale];
	NSString *country = [locale objectForKey:NSLocaleCountryCode];
	[lookupLocale selectItemWithTitle:country];

    [self openScanner];
	
}


#pragma mark Notification Handlers
// ================================================================================================== Notification Handlers
- (void)handleScannerWindowWillClose:(NSNotification *)n {
        
	// close QuickTime capture device
	[self closeVideoSession];
	
}

- (void)handleScannerWindowWillOpen:(NSNotification *)n {
    
	//[self openScanner];
	
}


#pragma mark Delegate Methods
// ================================================================================================== Delegate Methods
- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection {
	
    // Generate IplImage from CGImageRef
	frameCount++;
	if ((videoDevice) && (frameCount > 100)) { // we put in a delay so that it runs for a few seconds before processing.
		IplImage *input = [self createIplImageFromVideoBuffer:videoFrame];
		if ([self processIplImage:input]) {
            cvReleaseImage(&input);
        } else {
            cvReleaseImage(&input);
        }
	}
	return;

}

- (CIImage *)view:(QTCaptureView *)view willDisplayImage:(CIImage *)image {
	
	CIImage *flippedImage;
	
	// mirror the displayed image for the user so that it's more intuitive
	NSAffineTransform *flipTransform = [NSAffineTransform transform];
	CIFilter *flipFilter = [CIFilter filterWithName:@"CIAffineTransform"];;
	
	[flipTransform rotateByDegrees:180.0];
	[flipTransform scaleXBy:1.0 yBy:-1.0];
	
	[flipFilter setValue:flipTransform forKey:@"inputTransform"];
	[flipFilter setValue:image forKey:@"inputImage"];
	flippedImage = [flipFilter valueForKey:@"outputImage"];
	
	if (!keepScanning) {
		resetFrames++;
	} else {
		resetFrames = 0;
	}
	
	if ((resetFrames > 75) && (keepScanning == 0)) {
		[self scanAgain];
	}
	
	return flippedImage;
	
}


#pragma mark IBActions
// ================================================================================================== IBActions


#pragma mark Frame Grabbing & Processing
// ================================================================================================== Frame Grabbing & Processing
- (BOOL)processIplImage:(IplImage *)input {

	// Attempt to decode barcode.
	size_t width = input->width;
    size_t height = input->height; // / 2;
	
	if ((width > 0) && (height > 0)) {
		@autoreleasepool {
			NSString *digits = [NSString stringWithString:[BarCodeDecoder decode:input]];
			
			if ([digits length] > 0) {
				
				BOOL success = 0;
				
				switch ([fastAccurateSwitch selectedSegment]) {
					case 0:
					{
						// Accurate
                    if ([last1 isEqual:digits]) {
                        if ([last2 isEqual:digits]) {
                            success = 1;
                            self.last1 = nil;
                            self.last2 = nil;
                        } else {
                            self.last2 = digits;
                        }                
                    } else {
                        self.last2 = nil;
                        self.last1 = digits;
                    }
						
					}
						break;
						
					case 1:
					{
						// Fast
						if ([last1 isEqual:digits]) {
							success = 1;
                        self.last1 = nil;
						} else {
                        self.last1 = digits;
						}
					}
					default:
						break;
				}
				
				
				if (success && keepScanning) {
					if (keepScanning) {
						keepScanning = 0;
						[self foundBarcode:digits];
                    return YES;
					}
				}
			}
		
		}
        
	}

    return NO;

}

- (void)foundBarcode:(NSString *)aBarcode {
	
    NSLog(@"aBarcode: %@", aBarcode);
    
	[upcResult setStringValue:aBarcode];
	[barcodeLines setImage:[NSImage imageNamed:@"barcodeLinesBrightRed.png"]];

	NSMutableDictionary *lookup = [NSMutableDictionary dictionaryWithDictionary:[self amazonLookupResultsForBarcode:aBarcode]];
	
    NSLog(@"lookup: %@", lookup);
    
	if ([[lookup valueForKey:@"title"] length] > 0) {
		[barcodeLines setImage:[NSImage imageNamed:@"barcodeLinesGreen.png"]];
		[(NSSound *)[NSSound soundNamed:@"Glass"] play];
		[detailLabel setStringValue:[lookup valueForKey:@"title"]];
		[detailLabel setHidden:NO];
		[lookup setObject:aBarcode forKey:@"barcode"];
	} else {
		[(NSSound *)[NSSound soundNamed:@"Tink"] play];
		[barcodeLines setImage:[NSImage imageNamed:@"barcodeLinesRed.png"]];
	}
	
	keepScanning = 0;

}


#pragma mark Convenience Methods
// ================================================================================================== Convenience Methods
- (void)openScanner {
    
	runFrames = 0;
	pause = 0;
	
	captureSession = [[QTCaptureSession alloc] init];
	
	// ..................................................... open window
	[barcodeLines setImage:[NSImage imageNamed:@"barcodeLinesRed.png"]];
	[upcResult setStringValue:@""];
	[barcodeScannerWindow makeKeyAndOrderFront:self];
	
	
	// ..................................................... open QuickTime capture device
	NSError *error = nil;
	videoDevice = nil;
	videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
	
	if (videoDevice){
		[videoDevice open:&error];
	}
	
	if (error) {
		[QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeMuxed];
	}
	
	[videoDevice open:&error];
	
	if (error) {
		NSLog(@"Error: video device could not be re-opened after changing settings: %@", [error localizedDescription]);
		NSRunAlertPanel(@"No Camera Detected", @"Could not detect a camera, or it is in use by another application.", @"OK", nil, nil);
		[self closeVideoSession];
		return;
	}
	
	NSString *cameraDescription = [videoDevice modelUniqueID];
	NSString *cameraName = [videoDevice localizedDisplayName];
	
	if ((!cameraDescription) || (!cameraName)) {
		NSLog(@"Error: can't find video device");
		NSRunAlertPanel(@"No Camera Detected", @"Could not detect a camera, or it is in use by another application.", @"OK", nil, nil);
		[barcodeScannerWindow close];
		return;
	}
	
	// display the camera info in the UI
	[cameraNameLabel setStringValue:cameraDescription];
	
	
	// Focus code for FW iSight
	//[self setFWiSightShortFocus];
	
	
	// ..................................................... begin video stream
	QTCaptureDeviceInput *captureDeviceInput = [QTCaptureDeviceInput deviceInputWithDevice:videoDevice];
	[captureSession addInput:captureDeviceInput error:nil];
	[qtCaptureViewer setCaptureSession:captureSession];
    
    NSMutableDictionary *attributes = [NSMutableDictionary dictionaryWithObjectsAndKeys: 
                                       [NSNumber numberWithBool:YES], (id)kCVPixelBufferOpenGLCompatibilityKey,
                                       [NSNumber numberWithUnsignedInt:k2vuyPixelFormat], (id)kCVPixelBufferPixelFormatTypeKey,
                                       nil];
    
    
    // QTCaptureVideoPreviewOutput could be used if frame rate is not so important
    QTCaptureDecompressedVideoOutput *mCaptureDecompress = [[QTCaptureDecompressedVideoOutput alloc] init];
    //QTCaptureVideoPreviewOutput *mCaptureDecompress = [[QTCaptureVideoPreviewOutput alloc] init];
    [mCaptureDecompress setDelegate:self];
    [mCaptureDecompress setPixelBufferAttributes:attributes];
    if ([mCaptureDecompress respondsToSelector:@selector(setAutomaticallyDropsLateVideoFrames:)])
        [mCaptureDecompress setAutomaticallyDropsLateVideoFrames:YES]; //!!! 10.6 only
    BOOL success = [captureSession addOutput:mCaptureDecompress error:&error];
    if (!success) {
        NSLog(@"Error: could not add output device: %@", [error localizedDescription]);
        return;
    }
    
	[captureSession startRunning];
	
}

- (NSString *)queryAmazonForItemData:(NSString *)aBarcode {
	// Look up info via AWS
	NSString *httpRequest = [NSString stringWithFormat:@"%@&Operation=ItemSearch&Keywords=%@&SearchIndex=All&ItemPage=1&ResponseGroup=Small",
                             [self baseDomain], aBarcode];
	httpRequest = [self signRequest:httpRequest];
	
    
	NSError *e = nil;

	NSString *searchResults = [[NSString alloc] initWithContentsOfURL:[NSURL URLWithString:httpRequest]
															 encoding:NSASCIIStringEncoding
																error:&e];
    
	
	return searchResults;
	
    
}



#pragma mark Video Methods
// ================================================================================================== Video Methods
- (void)scanAgain {
	resetFrames = 0;
	keepScanning = 1;
	pause = 0;
	frameCount = 0;
	[barcodeLines setImage:[NSImage imageNamed:@"barcodeLinesRed.png"]];
}

- (BOOL)closeVideoSession {
		  
	NSError *error = nil;
	
	// end video session
	QTCaptureInput *input;
	for (input in [captureSession inputs]) {
		[captureSession removeInput:input];
	}
	[captureSession stopRunning];
	
	[videoDevice close];
	
	if (error) {
		return NO;
	}
	
	return YES;
}

// http://www.fourcc.org/fccyvrgb.php
// http://www.fourcc.org/yuv2ppm.c
// http://stackoverflow.com/questions/7954416/converting-yuv-into-bgr-or-rgb-in-opencv

- (IplImage *)createIplImageFromVideoBuffer:(CVImageBufferRef)pixelBuffer {
    	
    //8-bit 4:2:2 Component Y’CbCr format. Each 16 bit pixel is represented by an unsigned eight bit luminance component and two unsigned eight bit chroma components. Each pair of pixels shares a common set of chroma values. The components are ordered in memory; Cb, Y0, Cr, Y1. The luminance components have a range of [16, 235], while the chroma value has a range of [16, 240]. This is consistent with the CCIR601 spec. This format is fairly prevalent on both Mac and Win32 platforms. The equivalent Microsoft fourCC is ‘UYVY’.
    
    CVReturn possibleError = CVPixelBufferLockBaseAddress(pixelBuffer, 0);
	if (possibleError) {
		NSLog(@"Error locking pixel buffer, when looking for barcode.");
		return nil;
	}
	
	//[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
	
	Ptr pData = (Ptr)CVPixelBufferGetBaseAddress(pixelBuffer);
    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    //size_t rowBytes = CVPixelBufferGetBytesPerRow(pixelBuffer);
    
    IplImage *bgr_image = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
    
	NSInteger i, j, totalBytes = height * width * 2;
	for (i = 0, j = 0; i < totalBytes;  i+=6, j+=4) {
        
        //Use luminance only and save the calculations to RGB, this is a black and white picture that should have all we need.
        int y1 = (unsigned int)(pData[j+1] & 0xff);
        int y2 = (unsigned int)(pData[j+3] & 0xff);
        
        bgr_image->imageData[i] =  (int)y1;
        bgr_image->imageData[i+1] = (int)y1;
        bgr_image->imageData[i+2] = (int)y1;
        bgr_image->imageData[i+3] = (int)y2;
        bgr_image->imageData[i+4] = (int)y2;
        bgr_image->imageData[i+5] = (int)y2;
    }
    
	CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    
	// Need to scale this down so hi-res camera don't overload the decode code & cause crashes
	CvSize size = cvSize(640,480);
	IplImage *scaledSize = cvCreateImage(size, IPL_DEPTH_8U, 3);
	cvResize(bgr_image, scaledSize, CV_INTER_LINEAR);
	cvReleaseImage(&bgr_image);
	
    return scaledSize;
}

- (IplImage *)createIplImageFromCGImageRef:(CGImageRef)imageRef {
		
	// iOS: Get CGImage from UIImage
	//CGImageRef imageRef = image.CGImage;
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	// Creating temporal IplImage for drawing
	size_t width = CGImageGetWidth(imageRef);
    size_t height = CGImageGetHeight(imageRef); // / 2;
    
	IplImage *iplimage = cvCreateImage(
									   cvSize(width,height), IPL_DEPTH_8U, 4
									   );
	// Creating CGContext for temporal IplImage
	CGContextRef contextRef = CGBitmapContextCreate(
													iplimage->imageData, iplimage->width, iplimage->height,
													iplimage->depth, iplimage->widthStep,
													colorSpace, kCGImageAlphaPremultipliedLast|kCGBitmapByteOrderDefault
													);
	// Drawing CGImage to CGContext
	CGContextDrawImage(
					   contextRef,
					   CGRectMake(0, 0, width, height),
					   imageRef
					   );
	CGContextRelease(contextRef);
	CGColorSpaceRelease(colorSpace);
	
	// Creating result IplImage
	IplImage *ret = cvCreateImage(cvGetSize(iplimage), IPL_DEPTH_8U, 3);
	cvCvtColor(iplimage, ret, CV_RGBA2BGR);
	cvReleaseImage(&iplimage);
	
	return ret;
}


- (void)setFWiSightShortFocus {
	
    if ([videoDevice isOpen])
		[videoDevice close];
	
	// Original code from Wil Shipley... some errors, commenting out for now
	/*
	 long version;
	 OSErr result = Gestalt(gestaltQuickTime, &version); // check the version of QuickTime installed
	 if ((result == noErr) && (version >= 0x06408000)) { // we have version 6.4 -- so we can set autofocus off!
	 
	 VideoDigitizerComponent videoDigitizerComponent = SGGetVideoDigitizerComponent(sequenceGrabberChannel);
	 NSLog(@"VideoDigitizerComponent %x", videoDigitizerComponent);
	 
	 { // Dump the features to make sure we understand what's going on
	 long width, height;
	 err = VDGetPreferredImageDimensions(videoDigitizerComponent, &width, &height);
	 if (err == noErr)
	 NSLog(@"VDSetPreferredImageDimensions %d %d ", width, height);
	 else if (err != digiUnimpErr)
	 NoteErr(err);
	 
	 QTAtomContainer iidcFeaturesAtomContainer = NULL;
	 #define DUMP_ONE_FEATURE
	 #ifdef DUMP_ONE_FEATURE
	 err = VDIIDCGetFeaturesForSpecifier(videoDigitizerComponent, vdIIDCFeatureFocus, &iidcFeaturesAtomContainer);
	 #elif defined(DUMP_ALL_FEATURES)
	 err = VDIIDCGetFeatures(videoDigitizerComponent, &iidcFeaturesAtomContainer);
	 #else // dump default features
	 err = VDIIDCGetDefaultFeatures(videoDigitizerComponent, &iidcFeaturesAtomContainer);
	 #endif
	 NoteErr(err);
	 NSLog(@" VDIIDCGetFeaturesForSpecifier %d",iidcFeaturesAtomContainer);
	 if (iidcFeaturesAtomContainer != 0) {
	 
	 QTLockContainer(iidcFeaturesAtomContainer);
	 
	 unsigned long childIndex = QTCountChildrenOfType(iidcFeaturesAtomContainer, kParentAtomIsContainer, vdIIDCAtomTypeFeature);
	 NSLog(@" feature count %d",childIndex);
	 while (childIndex--) {
	 short ignoredIndex;
	 QTAtom currentFeatureAtom = QTFindChildByID(iidcFeaturesAtomContainer, kParentAtomIsContainer, vdIIDCAtomTypeFeature, childIndex+1, &ignoredIndex);
	 
	 printf("feature index %lu: ", childIndex+1);
	 
	 long dataSize = 0;
	 Ptr atomData = NULL;
	 
	 QTAtom currentFeatureTypeAndIDAtom = QTFindChildByID(iidcFeaturesAtomContainer, currentFeatureAtom, vdIIDCAtomTypeFeatureAtomTypeAndID, vdIIDCAtomIDFeatureAtomTypeAndID, &ignoredIndex);
	 printf("feature type %x\n", (unsigned int)currentFeatureTypeAndIDAtom);
	 err = QTGetAtomDataPtr(iidcFeaturesAtomContainer, currentFeatureTypeAndIDAtom, &dataSize, &atomData);
	 NoteErr(err);
	 VDIIDCFeatureAtomTypeAndID *featureAtomTypeAndID = (void *)atomData;
	 
	 printf(" feature '%c%c%c%c' of group '%c%c%c%c' has name '%s', settings in type '%c%c%c%c'\n", (char)(featureAtomTypeAndID->feature >> 24 & 0xff), (char)(featureAtomTypeAndID->feature >> 16 & 0xff), (char)(featureAtomTypeAndID->feature >> 8 & 0xff), (char)(featureAtomTypeAndID->feature >> 0 & 0xff), (char)(featureAtomTypeAndID->group >> 24 & 0xff), (char)(featureAtomTypeAndID->group >> 16 & 0xff), (char)(featureAtomTypeAndID->group >> 8 & 0xff), (char)(featureAtomTypeAndID->group >> 0 & 0xff), featureAtomTypeAndID->name, (char)(featureAtomTypeAndID->atomType >> 24 & 0xff), (char)(featureAtomTypeAndID->atomType >> 16 & 0xff), (char)(featureAtomTypeAndID->atomType >> 8 & 0xff), (char)(featureAtomTypeAndID->atomType >> 0 & 0xff));
	 
	 QTAtom currentFeatureSettingsAtom = QTFindChildByID(iidcFeaturesAtomContainer, currentFeatureAtom, featureAtomTypeAndID->atomType, featureAtomTypeAndID->atomID, &ignoredIndex);
	 printf(" feature atom %x\n", (unsigned int)currentFeatureSettingsAtom);
	 
	 dataSize = 0;
	 atomData = NULL;
	 err = QTGetAtomDataPtr(iidcFeaturesAtomContainer, currentFeatureSettingsAtom, &dataSize, &atomData);
	 NSLog(@"atomData %x, dataSize %d, struct size %d", atomData, dataSize, sizeof(VDIIDCFeatureSettings));
	 NoteErr(err);
	 if (featureAtomTypeAndID->atomType == vdIIDCAtomTypeFeatureSettings) {
	 NSParameterAssert(sizeof(VDIIDCFeatureSettings) == dataSize);
	 VDIIDCFeatureSettings *featureSettings = (void *)atomData;
	 
	 NSString *_dumpFlags(UInt32 flags) {
	 NSMutableString *newString = [NSMutableString string];
	 if (flags & vdIIDCFeatureFlagOn)
	 [newString appendString:@"On "];
	 if (flags & vdIIDCFeatureFlagOff)
	 [newString appendString:@"Off "];
	 if (flags & vdIIDCFeatureFlagManual)
	 [newString appendString:@"Manual "];
	 if (flags & vdIIDCFeatureFlagAuto)
	 [newString appendString:@"Auto "];
	 if (flags & vdIIDCFeatureFlagTune)
	 [newString appendString:@"Tune "];
	 if (flags & vdIIDCFeatureFlagRawControl)
	 [newString appendString:@"Raw "];
	 if (flags & vdIIDCFeatureFlagAbsoluteControl)
	 [newString appendString:@"Absolute "];
	 
	 return newString;
	 }
	 
	 printf(" capabilities: flags %x %s, rawMinimum %d , rawMaximum %d, absoluteMinimum %f, absoluteMaximum %f\n", (unsigned int)featureSettings->capabilities.flags, [_dumpFlags(featureSettings->capabilities.flags) cString], (int)(featureSettings->capabilities.rawMinimum), (int)(featureSettings->capabilities.rawMaximum), (float)(featureSettings->capabilities.absoluteMinimum), (float)(featureSettings->capabilities.absoluteMaximum));
	 printf(" state: flags %x %s, value %f\n", (unsigned int)featureSettings->state.flags, [_dumpFlags(featureSettings->state.flags) cString], (float)(featureSettings->state.value));
	 
	 } else if (featureAtomTypeAndID->atomType == vdIIDCAtomTypeFocusPointSettings) {
	 VDIIDCFocusPointSettings *focusPointSettings = (void *)atomData;
	 
	 printf(" state: focusPoint %d %d\n", focusPointSettings->focusPoint.v, focusPointSettings->focusPoint.h);
	 }
	 }
	 
	 QTUnlockContainer(iidcFeaturesAtomContainer);
	 }
	 }
	 }
	 */
	
	
	// Conor's code... doesn't seem to work on my iSight
	/* 
	 SGChannel           mChan;
	 SeqGrabComponent    mSeqGrab;
	 
	 OpenADefaultComponent(SeqGrabComponentType, 0, &mSeqGrab);
	 SGInitialize(mSeqGrab);
	 ComponentResult errorChannel = SGNewChannel(mSeqGrab, VideoMediaType, &mChan);
	 
	 if (errorChannel == 0) {
	 ComponentInstance vd = SGGetVideoDigitizerComponent(mChan);
	 if (vd) {
	 // SET the focus to be manual (at 10 out of 100)
	 QTAtomContainer iidcFeaturesAtomContainer = NULL;
	 QTNewAtomContainer(&iidcFeaturesAtomContainer);
	 
	 // 'feat'
	 QTAtom featureAtom = 0.0;
	 QTInsertChild(iidcFeaturesAtomContainer, kParentAtomIsContainer, vdIIDCAtomTypeFeature, 1, 0, 0, NULL, &featureAtom);
	 
	 // 't&id'
	 QTAtom typeAndIDAtom = 0.0;
	 VDIIDCFeatureAtomTypeAndID featureAtomTypeAndID = {vdIIDCFeatureFocus, vdIIDCGroupMechanics, {5, 'F', 'o', 'c', 'u', 's'}, vdIIDCAtomTypeFeatureSettings, vdIIDCAtomIDFeatureSettings};
	 QTInsertChild(iidcFeaturesAtomContainer, featureAtom, vdIIDCAtomTypeFeatureAtomTypeAndID, vdIIDCAtomIDFeatureAtomTypeAndID, 0, sizeof(featureAtomTypeAndID), &featureAtomTypeAndID, &typeAndIDAtom);
	 
	 // 'fstg'
	 QTAtom featureSettingsAtom = 0.0;
	 VDIIDCFeatureSettings featureSettings = {{0, 0, 0, 0.0, 0.0}, {vdIIDCFeatureFlagOn | vdIIDCFeatureFlagManual | vdIIDCFeatureFlagRawControl, 0.1}};
	 QTInsertChild(iidcFeaturesAtomContainer, featureAtom, vdIIDCAtomTypeFeatureSettings, vdIIDCAtomIDFeatureSettings, 0, sizeof(featureSettings), &featureSettings, &featureSettingsAtom);
	 
	 //VDIIDCSetFeatures(SGGetVideoDigitizerComponent(sequenceGrabberChannel), iidcFeaturesAtomContainer);
	 VDIIDCSetFeatures(vd, iidcFeaturesAtomContainer);
	 }
	 }
	 
	 SGDisposeChannel(mSeqGrab, mChan);
	 CloseComponent(mSeqGrab);
	 [NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]]; //Needed to give it time to close before using QTKit to acquire the iSight
	 
	 //Re open device
	 NSError *error;
	 BOOL success = [videoDevice open:&error];
	 
	 // Write error to the console log 
	 if (!success) {
	 videoDevice = nil;
	 NSLog(@"Error: video device could not be re-opened after changing settings: %@", [error localizedDescription]);
	 }	
	 
	 */
}


#pragma mark Amazon Lookup
// -------------------------------------------------------------------------- Amazon Lookup
- (NSDictionary *)amazonLookupResultsForBarcode:(NSString *)aBarcode {
    // First have to look up info via AWS
	NSString *httpRequest = [NSString stringWithFormat:@"%@&Operation=ItemSearch&Keywords=%@&SearchIndex=All&ItemPage=1&ResponseGroup=Small",
                             [self baseDomain], aBarcode];
	httpRequest = [self signRequest:httpRequest];
	
    
	NSError *e = nil;
	//NSMutableData *receivedData = [[NSURLConnection sendSynchronousRequest:theRequest returningResponse:nil error:&e] mutableCopy];
	NSString *searchResults = [[NSString alloc] initWithContentsOfURL:[NSURL URLWithString:httpRequest]
															 encoding:NSASCIIStringEncoding
																error:&e];
    
	NSDictionary *amazonDict = [XMLReader dictionaryForXMLString:searchResults error:e];
	
	if (!e) {
		return amazonDict;
		
	} else {
		return [NSDictionary dictionary];
	}
}

- (NSString *)encodeKeyword:(NSString *)keywords {
	// Amazon does not search for ’ it has to be '
	NSRange aRange = [keywords rangeOfString:[NSString stringWithFormat:@"%c", 213]];
	if (aRange.location != NSNotFound) {
		NSMutableString *removeApostraphe = [keywords mutableCopy];
		[removeApostraphe replaceCharactersInRange:aRange withString:@"'"];
		keywords = removeApostraphe;
	}
	
	// don't encode amp sign for power search but yes for regular search.
	if ([keywords rangeOfString:@"&Title"].location != NSNotFound
		|| [keywords rangeOfString:@"&Publisher"].location != NSNotFound
		|| [keywords rangeOfString:@"&Author"].location != NSNotFound
		|| [keywords rangeOfString:@"&Condition"].location != NSNotFound
		|| [keywords rangeOfString:@"&MinimumPrice"].location != NSNotFound
		|| [keywords rangeOfString:@"&MaximumPrice"].location != NSNotFound
		|| [keywords rangeOfString:@"&Power"].location != NSNotFound
		|| [keywords rangeOfString:@"&AudienceRating"].location != NSNotFound
		|| [keywords rangeOfString:@"&Actor"].location != NSNotFound
		|| [keywords rangeOfString:@"&Director"].location != NSNotFound
		|| [keywords rangeOfString:@"&Artist"].location != NSNotFound
		|| [keywords rangeOfString:@"&MusicLabel"].location != NSNotFound
		|| [keywords rangeOfString:@"&Brand"].location != NSNotFound
		|| [keywords rangeOfString:@"&Condition"].location != NSNotFound
		|| [keywords rangeOfString:@"&Manufacturer"].location != NSNotFound
		)

		return (NSString*)CFBridgingRelease(CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)keywords, (CFStringRef)@"=&", (CFStringRef)@",!$'()*+;@?\n\"<>#\t :/", kCFStringEncodingUTF8));
	else {
		if ([currentLocale isEqualToString:@"CA"]) // Last one with the old encodings
			return (NSString*)CFBridgingRelease(CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)keywords, NULL, (CFStringRef)@"=,!$&'()*+;@?\n\"<>#\t :/", kCFStringEncodingISOLatin1));
		else
			return (NSString*)CFBridgingRelease(CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)keywords, NULL, (CFStringRef)@"=,!$&'()*+;@?\n\"<>#\t :/", kCFStringEncodingUTF8));
	}
}

- (NSString *)HTTPRequestString:(NSString *)keywords {
	// ReviewSort=HelpfulVotes
	// &BrowseNode=133141011 Kindle Books
	
	return [NSString stringWithFormat:@"%@&Operation=ItemSearch&Keywords=%@&SearchIndex=All&ItemPage=1&ResponseGroup=Small", [self baseDomain], keywords];
}

- (NSString *)baseDomain {
	
	self.currentLocale = [lookupLocale titleOfSelectedItem];
	
	if ([self.currentLocale isEqualToString:@"US"]) {
        return [NSString stringWithFormat:@"http://ecs.amazonaws.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
	else if ([self.currentLocale isEqualToString:@"UK"]) {
        return [NSString stringWithFormat:@"http://ecs.amazonaws.co.uk/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
    else if ([self.currentLocale isEqualToString:@"DE"]) {
        return [NSString stringWithFormat:@"http://ecs.amazonaws.de/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
	else if ([self.currentLocale isEqualToString:@"JP"]) {
        return [NSString stringWithFormat:@"http://ecs.amazonaws.jp/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
	else if ([self.currentLocale isEqualToString:@"CA"]) {
        return [NSString stringWithFormat:@"http://ecs.amazonaws.ca/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
	else if ([self.currentLocale isEqualToString:@"FR"]) {
        return [NSString stringWithFormat:@"http://ecs.amazonaws.fr/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
    else if ([self.currentLocale isEqualToString:@"IT"]) {
        return [NSString stringWithFormat:@"http://webservices.amazon.it/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
    else if ([self.currentLocale isEqualToString:@"CN"]) {
        return [NSString stringWithFormat:@"http://webservices.amazon.cn/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
    else if ([self.currentLocale isEqualToString:@"ES"]) {
        return [NSString stringWithFormat:@"http://webservices.amazon.es/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
    }
	
	return [NSString stringWithFormat:@"http://ecs.amazonaws.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=%@&Version=2011-08-01&AssociateTag=%@", ACCESS_KEY, ASSOCIATE_TAG];
}

- (NSString *)signRequest:(NSString *)aRequest {
		
	NSInteger location = [aRequest rangeOfString:@"?"].location;
	NSString *baseDomain = [aRequest substringToIndex:location];
	NSString *valueSections = [aRequest substringFromIndex:location + 1];
	
	
	// Add time stamp   &Timestamp=2009-01-01T12:00:00Z
	if ([valueSections rangeOfString:@"&Timestamp"].location == NSNotFound) {
		
		NSDateFormatter *dateWriterFormatter = [[NSDateFormatter alloc] init];
		[dateWriterFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss'Z'"];
		[dateWriterFormatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
		NSString *aTime = [dateWriterFormatter stringFromDate:[NSDate date]];
		
		valueSections = [valueSections stringByAppendingFormat:@"&Timestamp=%@", aTime];
	}
    
	// Encode ,;:@ but do not double encode the %
	valueSections = (NSString*)CFBridgingRelease(CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)valueSections, (CFStringRef)@"&%", (CFStringRef)@":+,!$'()*;@?\n\"<>#\t/", kCFStringEncodingUTF8)); // ,';:@\"+?()! //valueSections = [NSMakeCollectable(CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)valueSections, NULL, (CFStringRef)@"+\"',:", kCFStringEncodingUTF8)) autorelease];
	
	// sort and rejoin
	NSArray *valuesArray = [valueSections componentsSeparatedByString:@"&"];
	valuesArray = [valuesArray sortedArrayUsingSelector:@selector(compare:)];
	valueSections = [valuesArray componentsJoinedByString:@"&"];
	
	// Create a string to sign
	NSString *baseWithoutHTTP = [baseDomain substringFromIndex:7];
	NSInteger slashLocation = [baseWithoutHTTP rangeOfString:@"/"].location;
	NSString *server = [baseWithoutHTTP substringToIndex:slashLocation];
	NSString *serverPath = [baseWithoutHTTP substringFromIndex:slashLocation];
	NSString *stringToSign = [NSString stringWithFormat:@"GET\n%@\n%@\n%@", server, serverPath, valueSections];
	
	//Sign the string and encode +=
	NSString *signature = [self hmacSHA256:stringToSign];
	signature = (NSString*)CFBridgingRelease(CFURLCreateStringByAddingPercentEscapes(NULL, (CFStringRef)signature, NULL, (CFStringRef)@"+=", kCFStringEncodingUTF8));
	
	// Append signature to sorted request
	aRequest = [baseDomain stringByAppendingFormat:@"?%@&Signature=%@", valueSections, signature];
	
	//NSLog(@"Signed: %@", aRequest);
	
	return aRequest;
}

- (NSString *)hmacSHA256:(NSString *)aString {
	
	const char *text = [aString UTF8String];
	int text_len = [aString lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
	const char *key = [SECRET_KEY UTF8String];
	int key_len = [SECRET_KEY lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
	
    //NSLog(@"Key: %@", secretKey);
	
	unsigned char k_ipad[CC_SHA256_BLOCK_BYTES + 1];    /* inner padding */
	unsigned char k_opad[CC_SHA256_BLOCK_BYTES + 1];    /* outer padding */
	unsigned char tk[CC_SHA256_DIGEST_LENGTH];
	int i;
	
	/* if key is longer than 64 bytes hash it*/
	if (key_len > CC_SHA256_BLOCK_BYTES) {
		
		CC_SHA256_CTX shaContext;
		CC_SHA256_Init(&shaContext);
		CC_SHA256_Update(&shaContext, key, key_len);
		CC_SHA256_Final(tk, &shaContext);
		key_len = CC_SHA256_DIGEST_LENGTH;
	}
	
	
	/* SHA256(K XOR opad, SHA256(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times
	 
	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected
	 */
	
	// start out by storing key in pads
	bzero( k_ipad, sizeof k_ipad);
	bzero( k_opad, sizeof k_opad);
	bcopy( key, k_ipad, key_len);
	bcopy( key, k_opad, key_len);
	
	// XOR key with ipad and opad values
	for (i=0; i<CC_SHA256_BLOCK_BYTES; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}
	
	// perform inner SHA256
	unsigned char hashedChars[CC_SHA256_DIGEST_LENGTH];
	CC_SHA256_CTX shaContext;
	CC_SHA256_Init(&shaContext);
	CC_SHA256_Update(&shaContext, k_ipad, CC_SHA256_BLOCK_BYTES);
	CC_SHA256_Update(&shaContext, text, text_len);
	CC_SHA256_Final(hashedChars, &shaContext);
	
	// perform outer SHA256
	CC_SHA256_Init(&shaContext);
	CC_SHA256_Update(&shaContext, k_opad, CC_SHA256_BLOCK_BYTES);
	CC_SHA256_Update(&shaContext, hashedChars, CC_SHA256_DIGEST_LENGTH);
	CC_SHA256_Final(hashedChars, &shaContext);
	
	
	NSData *hashData = [[NSData alloc] initWithBytes:hashedChars length:sizeof(hashedChars)];
	NSString *base64EncodedResult = [self base64StringFromData:hashData];
	
	return base64EncodedResult;
}

- (NSString *)base64StringFromData:(NSData *)data {
	if ([data length] == 0)
		return @"";
	
	const char encodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *characters = malloc((([data length] + 2) / 3) * 4);  //Freed by freeWhenDone below
	if (characters == NULL)
		return nil;
	NSUInteger length = 0;
	
	NSUInteger i = 0;
	while (i < [data length])
	{
		char buffer[3] = {0,0,0};
		short bufferLength = 0;
		while (bufferLength < 3 && i < [data length])
			buffer[bufferLength++] = ((char *)[data bytes])[i++];
		
		//  Encode the bytes in the buffer to four characters, including padding "=" characters if necessary.
		characters[length++] = encodingTable[(buffer[0] & 0xFC) >> 2];
		characters[length++] = encodingTable[((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xF0) >> 4)];
		if (bufferLength > 1)
			characters[length++] = encodingTable[((buffer[1] & 0x0F) << 2) | ((buffer[2] & 0xC0) >> 6)];
		else characters[length++] = '=';
		if (bufferLength > 2)
			characters[length++] = encodingTable[buffer[2] & 0x3F];
		else characters[length++] = '=';
	}
	
	return [[NSString alloc] initWithBytesNoCopy:characters length:length encoding:NSASCIIStringEncoding freeWhenDone:YES];
}


@end
