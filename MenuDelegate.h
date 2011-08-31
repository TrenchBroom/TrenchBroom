//
//  MenuDelegate.h
//  TrenchBroom
//
//  Created by Kristian Duske on 13.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MenuDelegate : NSObject <NSMenuDelegate> {
    IBOutlet NSMenuItem* showGridItem;
    IBOutlet NSMenuItem* snapToGridItem;
    IBOutlet NSMenuItem* gridSize8Item;
    IBOutlet NSMenuItem* gridSize16Item;
    IBOutlet NSMenuItem* gridSize32Item;
    IBOutlet NSMenuItem* gridSize64Item;
    IBOutlet NSMenuItem* gridSize128Item;
    IBOutlet NSMenuItem* gridSize256Item;
    IBOutlet NSMenuItem* moveFaceLeftItem;
    IBOutlet NSMenuItem* moveFaceLeftAltItem;
    IBOutlet NSMenuItem* moveFaceRightItem;
    IBOutlet NSMenuItem* moveFaceRightAltItem;
    IBOutlet NSMenuItem* moveFaceUpItem;
    IBOutlet NSMenuItem* moveFaceUpAltItem;
    IBOutlet NSMenuItem* moveFaceDownItem;
    IBOutlet NSMenuItem* moveFaceDownAltItem;
    IBOutlet NSMenuItem* rotateFaceLeftItem;
    IBOutlet NSMenuItem* rotateFaceLeftAltItem;
    IBOutlet NSMenuItem* rotateFaceRightItem;
    IBOutlet NSMenuItem* rotateFaceRightAltItem;
    IBOutlet NSMenuItem* stretchFaceHorizontallyItem;
    IBOutlet NSMenuItem* shrinkFaceHorizontallyItem;
    IBOutlet NSMenuItem* stretchFaceVerticallyItem;
    IBOutlet NSMenuItem* shrinkFaceVerticallyItem;
    IBOutlet NSMenuItem* duplicateItem;
    IBOutlet NSMenu* runMenu;
    IBOutlet NSMenuItem* runDefaultMenuItem;
    IBOutlet NSMenu* compileMenu;
    IBOutlet NSMenuItem* compileLastMenuItem;
}

- (IBAction)showPreferences:(id)sender;


@end
