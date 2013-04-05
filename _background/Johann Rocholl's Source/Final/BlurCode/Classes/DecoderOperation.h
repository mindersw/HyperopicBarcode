//
//  DecoderOperation.h
//  BlurCode
//
//  Created by Johann C. Rocholl on 3/1/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

#define HAVE_BOOL
#import "decoder.h"


@interface DecoderOperation : NSOperation {
	AppData *data;
	UIViewController *delegate;
}

@property AppData *data;
@property (nonatomic, retain) UIViewController *delegate;

@end
