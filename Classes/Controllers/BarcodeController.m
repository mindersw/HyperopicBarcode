//	(c) Copyright 2012 Conor Dearden & Michael LaMorte
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
//#import <QuickTime/QuickTime.h>
#import <QuartzCore/CIImage.h>
#import <QuartzCore/QuartzCore.h>
#import "BarCodeDecoder.h"

@implementation BarcodeController

@synthesize last1, last2;

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
		NSAutoreleasePool *innerPool = [NSAutoreleasePool new];
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
                    [innerPool release];
                    return YES;
				}
			}
		}
		
		[innerPool release];
        
	}

    return NO;

}

- (void)foundBarcode:(NSString *)aBarcode {
	
	[upcResult setStringValue:aBarcode];
	[barcodeLines setImage:[NSImage imageNamed:@"barcodeLinesBrightRed.png"]];

	NSMutableDictionary *lookup = [NSMutableDictionary dictionaryWithDictionary:[self webScrapeAmazonForBarcode:aBarcode]];
	
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

- (NSDictionary *)webScrapeAmazonForBarcode:(NSString *)aBarcode {
	
	NSString *country = [lookupLocale titleOfSelectedItem];
	
	NSError *e = nil;
	NSMutableString *searchResultsURL = [[NSMutableString alloc] initWithCapacity:10];
	
	if ([country isEqual:@"US"]) {
		[searchResultsURL setString:@"http://www.amazon.com/s/field-keywords="];
	} else if ([country isEqual:@"UK"]) {
		[searchResultsURL setString:@"http://www.amazon.co.uk/s/field-keywords="];
	} else if ([country isEqual:@"DE"]) {
		[searchResultsURL setString:@"http://www.amazon.de/s/field-keywords="];
	} else if ([country isEqual:@"JP"]) {
		[searchResultsURL setString:@"http://www.amazon.jp/s/field-keywords="];
	} else if ([country isEqual:@"CA"]) {
		[searchResultsURL setString:@"http://www.amazon.ca/s/field-keywords="];
	} else if ([country isEqual:@"FR"]) {
		[searchResultsURL setString:@"http://www.amazon.fr/s/field-keywords="];
	} else if ([country isEqual:@"IT"]) {
		[searchResultsURL setString:@"http://www.amazon.it/s/field-keywords="];
	} else if ([country isEqual:@"CN"]) {
		[searchResultsURL setString:@"http://www.amazon.cn/s/field-keywords="];
	} else if ([country isEqual:@"ES"]) {
		[searchResultsURL setString:@"http://www.amazon.es/s/field-keywords="];
	} else {
		[searchResultsURL setString:@"http://www.amazon.com/s/field-keywords="];
	}
	
	
	[searchResultsURL appendString:aBarcode];
	NSMutableString *searchResults = [[NSMutableString alloc] initWithContentsOfURL:[NSURL URLWithString:searchResultsURL] 
																		   encoding:NSUTF8StringEncoding 
																			  error:&e];
	if (e) {
		// we send the error home so that if there is a change in the URL, we'll know about it without the user sending a bug report
		NSString *logError = [NSString stringWithFormat:@"http://www.mindersoftworks.com/appsupport/mystuff2/errorlogger.php?error=%@&url=%@", [e localizedDescription], searchResults];
		NSURLRequest *theRequest = [NSURLRequest requestWithURL:[NSURL URLWithString:logError]];
		[NSURLConnection connectionWithRequest:theRequest delegate:self];
	}
	
	[searchResultsURL release];
	
	NSScanner *scanner;
	scanner = [NSScanner scannerWithString:(NSString *)searchResults];
	NSString *deleteMe = @"";
	
	if ([searchResults rangeOfString:@"result_0"].location == NSNotFound) {
		// no search results found
		[searchResults release];
		return [NSDictionary dictionary];
	}
	
	[scanner scanUpToString:@"result_0" intoString:&deleteMe];
	
	[searchResults deleteCharactersInRange:NSMakeRange(0, ([deleteMe length] - 1))];
	
	scanner = [NSScanner scannerWithString:(NSString *)searchResults];
	[scanner scanUpToString:@"href=\"" intoString:&deleteMe];
	[searchResults deleteCharactersInRange:NSMakeRange(0, ([deleteMe length] + 6))];
	
	scanner = [NSScanner scannerWithString:(NSString *)searchResults];	
	NSString *detailURL = @"";
	[scanner scanUpToString:@"/ref=" intoString:&detailURL];
	[searchResults release];
	
	
	NSString *detailPage = [[[NSString alloc] initWithContentsOfURL:[NSURL URLWithString:detailURL]
														  encoding:NSASCIIStringEncoding
															 error:&e] autorelease];
	
	if (e) {
		NSLog(@"webScrapeAmazonForBarcode Lookup Error: %@", e);
		return [NSDictionary dictionary];
	}
    if (detailPage == nil) {
		NSLog(@"webScrapeAmazonForBarcode Lookup Error details empty for: %@", searchResultsURL);
		return [NSDictionary dictionary];        
    }
    
	NSMutableString *mutableDetail = [NSMutableString stringWithString:detailPage];
	
	// --- Get Product Image
	if ([mutableDetail rangeOfString:@"prodImageCell"].location == NSNotFound) {
		// no results found
		return [NSDictionary dictionary];
	} else {
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([mutableDetail rangeOfString:@"prodImageCell"].location + 13))];
	}

	if ([mutableDetail rangeOfString:@"\"large\":\""].location == NSNotFound) {
		// no results found
		return [NSDictionary dictionary];
	} else {
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([mutableDetail rangeOfString:@"\"large\":\""].location + 9))];
	}

	
	
	scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
	NSString *productImageURL = @"";
	[scanner scanUpToString:@"\"" intoString:&productImageURL];
	
	[mutableDetail deleteCharactersInRange:NSMakeRange(0, [productImageURL length])];
	
	
	// --- Get Product Title //btAsinTitle
	NSString *productTitle = @"";
	//[scanner scanUpToString:@"btAsinTitle\">" intoString:&deleteMe];
	[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([mutableDetail rangeOfString:@"btAsinTitle\""].location + 13))];
	if ([[mutableDetail substringToIndex:1] isEqual:@">"]) {
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, 1)];
	}
	scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
	[scanner scanUpToString:@"</span>" intoString:&productTitle];
	scanner = [NSScanner scannerWithString:productTitle];
	[scanner scanUpToString:@"<span" intoString:&productTitle];
	if ([[productTitle substringToIndex:1] isEqual:@">"]) {
		productTitle = [productTitle substringFromIndex:1];
	}
	
	
	[mutableDetail deleteCharactersInRange:NSMakeRange(0, [productTitle length])];
	scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
	
	// --- Get Product Price
	//[mutableDetail setString:detailPage];
	NSMutableString *currency = [[[NSMutableString alloc] initWithString:@""] autorelease];
	NSString *listPrice = @"";
	
	if ([detailPage rangeOfString:@"class=\"listprice\">"].location != NSNotFound) {
		[scanner scanUpToString:@"class=\"listprice\">" intoString:&deleteMe];
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([deleteMe length] + 18))];
		scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
		NSString *price = @"";
		[scanner scanUpToString:@"<" intoString:&price];
		if ([price length] > 0) {
			[currency setString:[price substringToIndex:1]];
			[currency appendString:@" - "];
			listPrice = [price substringWithRange:NSMakeRange(1, ([price length] - 1))];
		}
		// get currency code
		[mutableDetail setString:detailPage];
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([detailPage rangeOfString:@"\"currenyCode\":\""].location + 15))];
		[currency appendString:[mutableDetail substringWithRange:NSMakeRange(0, 3)]];

		//NSLog(@"currency: %@", currency);
	}
	
	
	// --- Get Product Category
	NSString *category = @"";
	[mutableDetail setString:detailPage];
	// Not sure why the scanner isn't working
	//scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
	//[scanner scanUpToString:@"selected=\"selected\" current=\"parent\">" intoString:&deleteMe];
	if ([detailPage rangeOfString:@"selected=\"selected\" current=\"parent\">"].location != NSNotFound) {
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([detailPage rangeOfString:@"selected=\"selected\" current=\"parent\">"].location + 37))];
		scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
		[scanner scanUpToString:@"<" intoString:&category];
	} else if ([detailPage rangeOfString:@"selected=\"selected\">"].location != NSNotFound) {
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([detailPage rangeOfString:@"selected=\"selected\">"].location + 20))];
		scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
		[scanner scanUpToString:@"<" intoString:&category];
	}
	
	
	// --- Get Model Number (if available)
	NSString *model = @"";
	[mutableDetail setString:detailPage];
	if ([detailPage rangeOfString:@"<b>Item model number:</b>"].location != NSNotFound) {
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([detailPage rangeOfString:@"<b>Item model number:</b>"].location + 25))];
		scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
		[scanner scanUpToString:@"<" intoString:&model];
	}
	
	// --- Get Brand (if available)
	NSString *brand = @"";
	[mutableDetail setString:detailPage];
	if ([detailPage rangeOfString:@"field-brandtextbin="].location != NSNotFound) {
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([detailPage rangeOfString:@"field-brandtextbin="].location + 19))];
		[mutableDetail deleteCharactersInRange:NSMakeRange(0, ([mutableDetail rangeOfString:@">"].location + 1))];		
		scanner = [NSScanner scannerWithString:(NSString *)mutableDetail];
		[scanner scanUpToString:@"<" intoString:&brand];
	}
		
	NSDictionary *returnDict = [NSDictionary dictionaryWithObjectsAndKeys:productImageURL, @"imagePath", productTitle, @"title", currency, @"currency", listPrice, @"listPrice", category, @"category", model, @"model", brand, @"brand", nil];
	
	//NSLog(@"Return: %@", returnDict);
	
	return returnDict;
}


#pragma mark Convenience Methods
// ================================================================================================== Convenience Methods
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
		NSLog(@"Error locking pixel bufffer, when looking for barcode.");
		return nil;
	}
		
	Ptr pData = (Ptr)CVPixelBufferGetBaseAddress(pixelBuffer); 
    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    //size_t rowBytes = CVPixelBufferGetBytesPerRow(pixelBuffer);    
    
    IplImage *bgr_image = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
    
	NSInteger i, j, totalBytes = height * width * 2;
	for (i = 0, j = 0; i < totalBytes;  i+=6, j+=4) { 
        
        //Use lumnance only and save the calcualtions to RGB, this is a black and white picture that should have all we need.
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
    
    return bgr_image;
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

@end
