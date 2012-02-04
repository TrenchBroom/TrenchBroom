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

#import "MenuDelegate.h"
#import "MapWindowController.h"
#import "SelectionManager.h"
#import "Options.h"
#import "Grid.h"
#import "PreferencesController.h"
#import "CompilerProfileManager.h"
#import "CompilerProfile.h"
#import "ControllerUtils.h"
#import "PreferencesManager.h"

@implementation MenuDelegate

- (void)menuNeedsUpdate:(NSMenu *)menu {
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* keyWindow = [app keyWindow];
    NSWindowController* controller = [keyWindow windowController];
    if ([controller isKindOfClass:[MapWindowController class]]) {
        MapWindowController* mapController = (MapWindowController *)controller;
        Options* options = [mapController options];
        Grid* grid = [options grid];

        [textureLockItem setState:[options lockTextures] ? NSOnState : NSOffState];
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
    
    // build compile menu
    [compileMenu removeAllItems];
    CompilerProfileManager* profileManager = [CompilerProfileManager sharedManager];
    
    NSEnumerator* profileEn = [[profileManager profiles] objectEnumerator];
    CompilerProfile* profile;
    int index = 0;
    while ((profile = [profileEn nextObject])) {
        if ([profile name] != nil) {
            NSMenuItem* profileMenuItem = [[NSMenuItem alloc] initWithTitle:[profile name] action:@selector(compile:) keyEquivalent:@""];
            [profileMenuItem setTag:index];
            [compileMenu addItem:profileMenuItem];
            [profileMenuItem release];
        }
        
        index += 1;
    }
    
    // build run menu
    [runMenu removeAllItems];
    updateMenuWithExecutables(runMenu, NO, @selector(run:));
    
    // set default item titles
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    int lastCompilerProfileIndex = [preferences lastCompilerProfileIndex];
    if (lastCompilerProfileIndex >= 0 && lastCompilerProfileIndex < [[profileManager profiles] count]) {
        CompilerProfile* lastProfile = [[profileManager profiles] objectAtIndex:lastCompilerProfileIndex];
        [compileLastMenuItem setTitle:[NSString stringWithFormat:@"Compile %@", [lastProfile name]]];
    } else {
        [compileLastMenuItem setTitle:@"Compile Last Profile"];
    }
    
    NSString* quakePath = [preferences quakePath];
    NSString* quakeExecutable = [preferences quakeExecutable];
    if (quakePath != nil && quakeExecutable != nil) {
        [runDefaultMenuItem setTitle:[NSString stringWithFormat:@"Run %@", [quakeExecutable stringByDeletingPathExtension]]];
    } else {
        [runDefaultMenuItem setTitle:@"Run Default Engine"];
    }
}

- (IBAction)showPreferences:(id)sender {
    PreferencesController* preferencesController = [PreferencesController sharedPreferences];
    [[preferencesController window] makeKeyAndOrderFront:self];
}

@end
