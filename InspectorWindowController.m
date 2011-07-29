//
//  InspectorWindowController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
