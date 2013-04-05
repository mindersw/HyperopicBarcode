//
//  FullscreenPickerController.h
//  BlurCode
//
//  Created by Johann C. Rocholl on 2/19/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface FullscreenPickerController : UIImagePickerController {
	NSTimer *captureTimer;
}

@property (nonatomic, retain) NSTimer *captureTimer;

@end
