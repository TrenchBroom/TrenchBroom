//
//  PreferencesWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface PreferencesController : NSWindowController {
    IBOutlet NSTextField* quakePathTextField;
}

+ (PreferencesController *)sharedPreferences;

- (IBAction)chooseQuakePath:(id)sender;

@end
