/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>

@class PreferencesManager;
@class CompilerProfileManager;
@class PreferencesViewAnimation;

@interface PreferencesController : NSWindowController {
    IBOutlet NSTextField* quakePathTextField;
    IBOutlet NSPopUpButton* quakeExecutablePopUp;
    IBOutlet NSToolbar* toolbar;
    IBOutlet NSToolbarItem* generalToolbarItem;
    IBOutlet NSToolbarItem* compilerToolbarItem;
    IBOutlet NSView* generalView;
    IBOutlet NSView* compilerView;
    IBOutlet NSArrayController* compilerToolsArrayController;
    NSMutableDictionary* toolbarItemToViewMap;
    PreferencesViewAnimation* currentAnimation;
    NSString* lastSelectedIdentifier;
}

+ (PreferencesController *)sharedPreferences;

- (IBAction)chooseQuakePath:(id)sender;
- (PreferencesManager *)preferences;
- (CompilerProfileManager *)compilerProfileManager;

- (IBAction)generalToolbarItemSelected:(id)sender;
- (IBAction)compilerToolbarItemSelected:(id)sender;

@end
