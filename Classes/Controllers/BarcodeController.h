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
#import <AVFoundation/AVFoundation.h>
#import "BarCodeDecoder.h"

@interface BarcodeController : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property (strong, nonatomic) NSString *last1;
@property (strong, nonatomic) NSString *last2;
@property (readwrite, strong) NSString *currentLocale;

// ================================================================= Delegate Methods
- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection;

// ================================================================= Frame Grabbing & Processing
- (void)beginCapture;
- (BOOL)processIplImage:(IplImage *)input;
- (void)foundBarcode:(NSString *)aBarcode;


// ================================================================= Convenience Methods
- (NSString *)queryAmazonForItemData:(NSString *)aBarcode;
- (BOOL)closeVideoSession;
- (IplImage *)createIplImageFromVideoBuffer:(CVImageBufferRef)pixelBuffer;
- (IplImage *)createIplImageFromCGImageRef:(CGImageRef)imageRef;


// ================================================================= Amazon Lookup
- (NSDictionary *)amazonLookupResultsForBarcode:(NSString *)aBarcode;
- (NSString *)encodeKeyword:(NSString *)keywords;
- (NSString *)HTTPRequestString:(NSString *)keywords;
- (NSString *)baseDomain;
- (NSString *)signRequest:(NSString *)aRequest;
- (NSString *)hmacSHA256:(NSString *)aString;
- (NSString *)base64StringFromData:(NSData *)data;


@end
