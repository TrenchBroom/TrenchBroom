//
//  PreferencesWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PreferencesManager;
@class CompilerManager;

@interface PreferencesController : NSWindowController {
    IBOutlet NSTextField* quakePathTextField;
    IBOutlet NSPopUpButton* quakeExecutablePopUp;
    IBOutlet NSToolbar* toolbar;
    IBOutlet NSToolbarItem* generalToolbarItem;
    IBOutlet NSToolbarItem* compilerToolbarItem;
    IBOutlet NSView* generalView;
    IBOutlet NSView* compilerView;
    NSMutableDictionary* toolbarItemToViewMap;
}

+ (PreferencesController *)sharedPreferences;

- (IBAction)chooseQuakePath:(id)sender;
- (PreferencesManager *)preferences;
- (CompilerManager *)compilerManager;

- (IBAction)generalToolbarItemSelected:(id)sender;
- (IBAction)compilerToolbarItemSelected:(id)sender;
@end
