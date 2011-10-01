//
//  AppDelegate.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PreferencesManager;

@interface AppDelegate : NSObject {

}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender;

- (PreferencesManager *)preferences;

@end
