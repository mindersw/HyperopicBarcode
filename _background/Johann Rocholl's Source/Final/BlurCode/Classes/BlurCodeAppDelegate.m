//
//  BlurCodeAppDelegate.m
//  BlurCode
//
//  Created by Johann C. Rocholl on 2/19/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import "BlurCodeAppDelegate.h"
#import "BlurCodeViewController.h"
#import "FullscreenPickerController.h"

@implementation BlurCodeAppDelegate

@synthesize window;
@synthesize viewController;


- (void)applicationDidFinishLaunching:(UIApplication *)application {    
    
    // Override point for customization after app launch    
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];
	
    FullscreenPickerController* picker = [[[FullscreenPickerController alloc] init] autorelease];
    picker.sourceType = UIImagePickerControllerSourceTypeCamera;    
    picker.allowsImageEditing = NO;
	// picker.delegate = self;
    [viewController presentModalViewController:picker animated:NO];
}


- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    [picker dismissModalViewControllerAnimated:NO];
}


- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}


@end
