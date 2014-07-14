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

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import "BarCodeDecoder.h"

@class QTCaptureSession, QTCaptureView, QTCaptureDevice;

@interface BarcodeController : NSObject {

	// ================================================================= IBOutlets
	IBOutlet NSWindow *barcodeScannerWindow;
	IBOutlet NSImageView *barcodeLines;
	IBOutlet QTCaptureView *qtCaptureViewer;
	IBOutlet NSTextField *upcResult;
	IBOutlet NSTextField *cameraNameLabel;
	IBOutlet NSTextField *detailLabel;
	IBOutlet NSSegmentedControl *fastAccurateSwitch;
	IBOutlet NSPopUpButton *lookupLocale;
	
	// ================================================================= ivars
	QTCaptureSession *captureSession;
	QTCaptureDevice *videoDevice;
	NSString *currentLocale;
	
	// ================================================================= Barcode Processing
	int numberOfDigitsFound;
	int frameCount;
	int runFrames;
	int resetFrames;
	
	// for Faster/Better switch
	NSString *last1;
	NSString *last2;
	
	BOOL keepScanning, pause;
	
	//stores most recent frame grabbed from a CVImageBufferRef
	CVImageBufferRef currentImageBuffer;
	
	BarCodeDecoder *decoder;
}

@property (strong, nonatomic) NSString *last1;
@property (strong, nonatomic) NSString *last2;
@property (readwrite, strong) NSString *currentLocale;

// ================================================================= Delegate Methods
- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection;
- (CIImage *)view:(QTCaptureView *)view willDisplayImage:(CIImage *)image;


// ================================================================= Frame Grabbing & Processing
//- (void)grabFrameAndProcess:(NSBitmapImageRep *)bitmap;
- (BOOL)processIplImage:(IplImage *)input;
//- (void)processCGImageRef:(CGImageRef)imgRef;
- (void)foundBarcode:(NSString *)aBarcode;


// ================================================================= Convenience Methods
- (void)openScanner;
- (void)scanAgain;
- (BOOL)closeVideoSession;
- (IplImage *)createIplImageFromVideoBuffer:(CVImageBufferRef)pixelBuffer;
- (IplImage *)createIplImageFromCGImageRef:(CGImageRef)imageRef;
- (void)setFWiSightShortFocus;


// ================================================================= Amazon Lookup


@end
