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
            [moveFaceLeftItem setTitle:[NSString stringWithFormat:@"Move Left By %i", [options gridSize]]];
            [moveFaceLeftAltItem setTitle:@"Move Left By 1"];
            [moveFaceRightItem setTitle:[NSString stringWithFormat:@"Move Right By %i", [options gridSize]]];
            [moveFaceRightAltItem setTitle:@"Move Right By 1"];
            [moveFaceUpItem setTitle:[NSString stringWithFormat:@"Move Up By %i", [options gridSize]]];
            [moveFaceUpAltItem setTitle:@"Move Up By 1"];
            [moveFaceDownItem setTitle:[NSString stringWithFormat:@"Move Down By %i", [options gridSize]]];
            [moveFaceDownAltItem setTitle:@"Move Down By 1"];
            [rotateFaceLeftItem setTitle:@"Rotate Left By 15"];
            [rotateFaceLeftAltItem setTitle:@"Rotate Left By 1"];
            [rotateFaceRightItem setTitle:@"Rotate Right By 15"];
            [rotateFaceRightAltItem setTitle:@"Rotate Right By 1"];
        } else {
            [moveFaceLeftItem setTitle:@"Move Left By 1"];
            [moveFaceLeftAltItem setTitle:[NSString stringWithFormat:@"Move Left By %i", [options gridSize]]];
            [moveFaceRightItem setTitle:@"Move Right By 1"];
            [moveFaceRightAltItem setTitle:[NSString stringWithFormat:@"Move Right By %i", [options gridSize]]];
            [moveFaceUpItem setTitle:@"Move Up By 1"];
            [moveFaceUpAltItem setTitle:[NSString stringWithFormat:@"Move Up By %i", [options gridSize]]];
            [moveFaceDownItem setTitle:@"Move Down By 1"];
            [moveFaceDownAltItem setTitle:[NSString stringWithFormat:@"Move Down By %i", [options gridSize]]];
            [rotateFaceLeftItem setTitle:@"Rotate Left By 1"];
            [rotateFaceLeftAltItem setTitle:@"Rotate Left By 15"];
            [rotateFaceRightItem setTitle:@"Rotate Right By 1"];
            [rotateFaceRightAltItem setTitle:@"Rotate Right By 15"];
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
