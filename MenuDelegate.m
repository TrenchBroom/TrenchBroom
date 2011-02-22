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
        [snapToGridItem setEnabled:YES];
        [gridSize8Item setEnabled:YES];
        [gridSize16Item setEnabled:YES];
        [gridSize32Item setEnabled:YES];
        [gridSize64Item setEnabled:YES];
        [gridSize128Item setEnabled:YES];
        [gridSize256Item setEnabled:YES];
        
        [showGridItem setState:[options drawGrid] ? NSOnState : NSOffState];
        [snapToGridItem setState:[options snapToGrid] ? NSOnState : NSOffState];
        [gridSize8Item setState:[options gridSize] == 8 ? NSOnState : NSOffState];
        [gridSize16Item setState:[options gridSize] == 16 ? NSOnState : NSOffState];
        [gridSize32Item setState:[options gridSize] == 32 ? NSOnState : NSOffState];
        [gridSize64Item setState:[options gridSize] == 64 ? NSOnState : NSOffState];
        [gridSize128Item setState:[options gridSize] == 128 ? NSOnState : NSOffState];
        [gridSize256Item setState:[options gridSize] == 256 ? NSOnState : NSOffState];
        
        if ([options snapToGrid]) {
            [moveFaceLeftItem setTitle:@"Move Left"];
            [moveFaceLeftAltItem setTitle:@"Move Left (No Snap)"];
            [moveFaceRightItem setTitle:@"Move Right"];
            [moveFaceRightAltItem setTitle:@"Move Right (No Snap)"];
            [moveFaceUpItem setTitle:@"Move Up"];
            [moveFaceUpAltItem setTitle:@"Move Up (No Snap)"];
            [moveFaceDownItem setTitle:@"Move Down"];
            [moveFaceDownAltItem setTitle:@"Move Down (No Snap)"];
            [rotateFaceLeftItem setTitle:@"Rotate Left"];
            [rotateFaceLeftAltItem setTitle:@"Rotate Left (No Snap)"];
            [rotateFaceRightItem setTitle:@"Rotate Right"];
            [rotateFaceRightAltItem setTitle:@"Rotate Right (No Snap)"];
        } else {
            [moveFaceLeftItem setTitle:@"Move Left (No Snap)"];
            [moveFaceLeftAltItem setTitle:@"Move Left"];
            [moveFaceRightItem setTitle:@"Move Right (No Snap)"];
            [moveFaceRightAltItem setTitle:@"Move Right"];
            [moveFaceUpItem setTitle:@"Move Up (No Snap)"];
            [moveFaceUpAltItem setTitle:@"Move Up"];
            [moveFaceDownItem setTitle:@"Move Down (No Snap)"];
            [moveFaceDownAltItem setTitle:@"Move Down"];
            [rotateFaceLeftItem setTitle:@"Rotate Left (No Snap)"];
            [rotateFaceLeftAltItem setTitle:@"Rotate Left"];
            [rotateFaceRightItem setTitle:@"Rotate Right (No Snap)"];
            [rotateFaceRightAltItem setTitle:@"Rotate Right"];
        }
    } else {
        [showGridItem setEnabled:NO];
        [snapToGridItem setEnabled:NO];
        [gridSize8Item setEnabled:NO];
        [gridSize16Item setEnabled:NO];
        [gridSize32Item setEnabled:NO];
        [gridSize64Item setEnabled:NO];
        [gridSize128Item setEnabled:NO];
        [gridSize256Item setEnabled:NO];
        
        [showGridItem setState:NSOffState];
        [snapToGridItem setState:NSOffState];
        [gridSize8Item setState:NSOffState];
        [gridSize16Item setState:NSOffState];
        [gridSize32Item setState:NSOffState];
        [gridSize64Item setState:NSOffState];
        [gridSize128Item setState:NSOffState];
        [gridSize256Item setState:NSOffState];
    }
    
    
}

@end
