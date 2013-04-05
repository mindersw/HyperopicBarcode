//
//  BlurCodeAppDelegate.h
//  BlurCode
//
//  Created by Johann C. Rocholl on 2/19/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import <UIKit/UIKit.h>

@class BlurCodeViewController;

@interface BlurCodeAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    BlurCodeViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet BlurCodeViewController *viewController;

@end

