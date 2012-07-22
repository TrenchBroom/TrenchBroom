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

#import "PreferencesWindowController.h"
#import "Model/Preferences.h"

using namespace TrenchBroom::Model;
using namespace TrenchBroom::Controller;

namespace TrenchBroom {
    namespace Controller {
        PreferencesListener::PreferencesListener() {
            Preferences::sharedPreferences->preferencesDidChange += new Preferences::PreferencesEvent::Listener<PreferencesListener>(this, &PreferencesListener::preferencesDidChange);
            }
        
        PreferencesListener::~PreferencesListener() {
            if (Preferences::sharedPreferences != NULL)
                Preferences::sharedPreferences->preferencesDidChange -= new Preferences::PreferencesEvent::Listener<PreferencesListener>(this, &PreferencesListener::preferencesDidChange);
        }
        
        void PreferencesListener::preferencesDidChange(const std::string& key) {
            [[PreferencesWindowController sharedInstance] update];
        }
    }
}

static PreferencesWindowController* sharedInstance = nil;
static PreferencesListener* preferencesListener = nil;

@interface PreferencesWindowController (Private)

- (NSString *)nsString:(const std::string&)cppString;

@end

@implementation PreferencesWindowController (Private)

- (NSString *)nsString:(const std::string&)cppString {
    return [NSString stringWithCString:cppString.c_str() encoding:NSASCIIStringEncoding];
}

@end

@implementation PreferencesWindowController

+ (PreferencesWindowController *)sharedInstance {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
        if (preferencesListener == nil)
            preferencesListener = new PreferencesListener();
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (oneway void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (void)dealloc {
    if (preferencesListener == nil)
        delete preferencesListener;
    [super dealloc];
}

- (NSString *)windowNibName {
    return @"PreferencesWindow";
}
- (void)windowDidLoad {
    [super windowDidLoad];
    [self update];
}

- (void)update {
    Preferences& prefs = *Preferences::sharedPreferences;
    
    [quakePathLabel setStringValue:[self nsString:prefs.quakePath()]];
    
    [brightnessSlider setFloatValue:prefs.brightness()];
    [brightnessLabel setFloatValue:prefs.brightness()];
    [fovSlider setFloatValue:prefs.cameraFov()];
    [fovLabel setFloatValue:prefs.cameraFov()];
    
    [cameraLookSpeedSlider setFloatValue:prefs.cameraLookSpeed()];
    [cameraLookSpeedLabel setFloatValue:prefs.cameraLookSpeed()];
    [cameraLookInvertYCheckbox setState:prefs.cameraLookInvertY() ? NSOnState : NSOffState];
    [cameraPanSpeedSlider setFloatValue:prefs.cameraPanSpeed()];
    [cameraPanSpeedLabel setFloatValue:prefs.cameraPanSpeed()];
    [cameraPanInvertXCheckbox setState:prefs.cameraPanInvertX() ? NSOnState : NSOffState];
    [cameraPanInvertYCheckbox setState:prefs.cameraPanInvertY() ? NSOnState : NSOffState];
    [cameraMoveSpeedSlider setFloatValue:prefs.cameraMoveSpeed()];
    [cameraMoveSpeedLabel setFloatValue:prefs.cameraMoveSpeed()];
}

- (IBAction)chooseQuakePath:(id)sender {
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setAllowsOtherFileTypes:NO];
    [openPanel setTitle:@"Choose Quake Path"];
    [openPanel setNameFieldLabel:@"Quake Path"];
    [openPanel setCanCreateDirectories:NO];
    
    if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
        for (NSURL* url in [openPanel URLs]) {
            NSString* quakePath = [url path];
            if (quakePath != nil) {
                Preferences::sharedPreferences->setQuakePath([quakePath cStringUsingEncoding:NSASCIIStringEncoding]);
            }
        }
    }
}

- (IBAction)changeBrightness:(id)sender {
    Preferences::sharedPreferences->setBrightness([brightnessSlider floatValue]);
}

- (IBAction)changeFov:(id)sender {
    Preferences::sharedPreferences->setCameraFov([fovSlider floatValue]);
}

- (IBAction)changeCameraLookSpeed:(id)sender {
    Preferences::sharedPreferences->setCameraLookSpeed([cameraLookSpeedSlider floatValue]);
}

- (IBAction)changeCameraLookInvertY:(id)sender {
    Preferences::sharedPreferences->setCameraLookInvertY([cameraLookInvertYCheckbox state] == NSOnState);
}

- (IBAction)changeCameraPanSpeed:(id)sender {
    Preferences::sharedPreferences->setCameraPanSpeed([cameraPanSpeedSlider floatValue]);
}

- (IBAction)changeCameraPanInvertX:(id)sender {
    Preferences::sharedPreferences->setCameraPanInvertX([cameraPanInvertXCheckbox state] == NSOnState);
}

- (IBAction)changeCameraPanInvertY:(id)sender {
    Preferences::sharedPreferences->setCameraPanInvertY([cameraPanInvertYCheckbox state] == NSOnState);
}

- (IBAction)changeCameraMoveSpeed:(id)sender {
    Preferences::sharedPreferences->setCameraMoveSpeed([cameraMoveSpeedSlider floatValue]);
}

@end
