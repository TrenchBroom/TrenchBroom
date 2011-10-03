/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "InspectorWindowController.h"
#import "InspectorViewController.h"
#import "MapWindowController.h"

static InspectorWindowController* sharedInstance = nil;

@implementation InspectorWindowController

+ (InspectorWindowController *)sharedInspector {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[InspectorWindowController alloc] initWithWindowNibName:@"InspectorWindow"];
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
    [inspectorViewController release];
    [super dealloc];
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    inspectorViewController = [[InspectorViewController alloc] initWithNibName:@"InspectorView" bundle:nil];
    NSView* inspectorView = [inspectorViewController view];
    
    NSWindow* window = [self window];
    [window setContentView:inspectorView];
    [inspectorViewController setMapWindowController:mapWindowController];
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController {
    mapWindowController = theMapWindowController;
    [inspectorViewController setMapWindowController:mapWindowController];
}

- (MapWindowController *)mapWindowController {
    return mapWindowController;
}

@end
