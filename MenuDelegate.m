//
//  MenuDelegate.m
//  TrenchBroom
//
//  Created by Kristian Duske on 13.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MenuDelegate.h"
#import "MapWindowController.h"
#import "Options.h"

@implementation MenuDelegate

- (void)menuNeedsUpdate:(NSMenu *)menu {
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* keyWindow = [app keyWindow];
    NSWindowController* controller = [keyWindow windowController];
    if ([controller isKindOfClass:[MapWindowController class]]) {
        MapWindowController* mapController = (MapWindowController *)controller;
        Options* options = [mapController options];
        
        [showGridItem setEnabled:YES];
        [gridSize8Item setEnabled:YES];
        [gridSize16Item setEnabled:YES];
        [gridSize32Item setEnabled:YES];
        [gridSize64Item setEnabled:YES];
        [gridSize128Item setEnabled:YES];
        [gridSize256Item setEnabled:YES];
        
        [showGridItem setState:[options drawGrid] ? NSOnState : NSOffState];
        [gridSize8Item setState:[options gridSize] == 8 ? NSOnState : NSOffState];
        [gridSize16Item setState:[options gridSize] == 16 ? NSOnState : NSOffState];
        [gridSize32Item setState:[options gridSize] == 32 ? NSOnState : NSOffState];
        [gridSize64Item setState:[options gridSize] == 64 ? NSOnState : NSOffState];
        [gridSize128Item setState:[options gridSize] == 128 ? NSOnState : NSOffState];
        [gridSize256Item setState:[options gridSize] == 256 ? NSOnState : NSOffState];
    } else {
        [showGridItem setEnabled:NO];
        [gridSize8Item setEnabled:NO];
        [gridSize16Item setEnabled:NO];
        [gridSize32Item setEnabled:NO];
        [gridSize64Item setEnabled:NO];
        [gridSize128Item setEnabled:NO];
        [gridSize256Item setEnabled:NO];
        
        [showGridItem setState:NSOffState];
        [gridSize8Item setState:NSOffState];
        [gridSize16Item setState:NSOffState];
        [gridSize32Item setState:NSOffState];
        [gridSize64Item setState:NSOffState];
        [gridSize128Item setState:NSOffState];
        [gridSize256Item setState:NSOffState];
    }
}

@end
