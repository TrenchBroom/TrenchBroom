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
#import "Utilities/Event.h"

namespace TrenchBroom {
    namespace Controller {
        class PreferencesListener {
        public:
            PreferencesListener();
            ~PreferencesListener();
            void preferencesDidChange(const string& key);
        };
    }
}

@interface PreferencesWindowController : NSWindowController {
    IBOutlet NSTextField* quakePathLabel;
    IBOutlet NSSlider* brightnessSlider;
    IBOutlet NSTextField* brightnessLabel;
    IBOutlet NSSlider* fovSlider;
    IBOutlet NSTextField* fovLabel;
    IBOutlet NSButton* invertMouseCheckbox;
}

+ (PreferencesWindowController *)sharedInstance;

- (void)update;
- (IBAction)chooseQuakePath:(id)sender;
- (IBAction)changeBrightness:(id)sender;
- (IBAction)changeFov:(id)sender;
- (IBAction)changeInvertMouse:(id)sender;

@end
