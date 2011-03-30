//
//  MenuDelegate.m
//  TrenchBroom
//
//  Created by Kristian Duske on 13.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MenuDelegate.h"
#import "MapWindowController.h"
#import "SelectionManager.h"
#import "Options.h"
#import "Grid.h"

@implementation MenuDelegate

- (void)menuNeedsUpdate:(NSMenu *)menu {
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* keyWindow = [app keyWindow];
    NSWindowController* controller = [keyWindow windowController];
    if ([controller isKindOfClass:[MapWindowController class]]) {
        MapWindowController* mapController = (MapWindowController *)controller;
        Options* options = [mapController options];
        Grid* grid = [options grid];

        [showGridItem setState:[grid draw] ? NSOnState : NSOffState];
        [snapToGridItem setState:[grid snap] ? NSOnState : NSOffState];
        [gridSize8Item setState:[grid size] == 0 ? NSOnState : NSOffState];
        [gridSize16Item setState:[grid size] == 1 ? NSOnState : NSOffState];
        [gridSize32Item setState:[grid size] == 2 ? NSOnState : NSOffState];
        [gridSize64Item setState:[grid size] == 3 ? NSOnState : NSOffState];
        [gridSize128Item setState:[grid size] == 4 ? NSOnState : NSOffState];
        [gridSize256Item setState:[grid size] == 5 ? NSOnState : NSOffState];
        
        if ([grid snap]) {
            [moveFaceLeftItem setTitle:[NSString stringWithFormat:@"Move Left By %i", [[options grid] size]]];
            [moveFaceLeftAltItem setTitle:@"Move Left By 1"];
            [moveFaceRightItem setTitle:[NSString stringWithFormat:@"Move Right By %i", [[options grid] size]]];
            [moveFaceRightAltItem setTitle:@"Move Right By 1"];
            [moveFaceUpItem setTitle:[NSString stringWithFormat:@"Move Up By %i", [[options grid] size]]];
            [moveFaceUpAltItem setTitle:@"Move Up By 1"];
            [moveFaceDownItem setTitle:[NSString stringWithFormat:@"Move Down By %i", [[options grid] size]]];
            [moveFaceDownAltItem setTitle:@"Move Down By 1"];
            [rotateFaceLeftItem setTitle:@"Rotate Left By 15"];
            [rotateFaceLeftAltItem setTitle:@"Rotate Left By 1"];
            [rotateFaceRightItem setTitle:@"Rotate Right By 15"];
            [rotateFaceRightAltItem setTitle:@"Rotate Right By 1"];
        } else {
            [moveFaceLeftItem setTitle:@"Move Left By 1"];
            [moveFaceLeftAltItem setTitle:[NSString stringWithFormat:@"Move Left By %i", [[options grid] size]]];
            [moveFaceRightItem setTitle:@"Move Right By 1"];
            [moveFaceRightAltItem setTitle:[NSString stringWithFormat:@"Move Right By %i", [[options grid] size]]];
            [moveFaceUpItem setTitle:@"Move Up By 1"];
            [moveFaceUpAltItem setTitle:[NSString stringWithFormat:@"Move Up By %i", [[options grid] size]]];
            [moveFaceDownItem setTitle:@"Move Down By 1"];
            [moveFaceDownAltItem setTitle:[NSString stringWithFormat:@"Move Down By %i", [[options grid] size]]];
            [rotateFaceLeftItem setTitle:@"Rotate Left By 1"];
            [rotateFaceLeftAltItem setTitle:@"Rotate Left By 15"];
            [rotateFaceRightItem setTitle:@"Rotate Right By 1"];
            [rotateFaceRightAltItem setTitle:@"Rotate Right By 15"];
        }
    } else {
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
