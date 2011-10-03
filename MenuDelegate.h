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
    IBOutlet NSMenuItem* textureLockItem;
    IBOutlet NSMenuItem* duplicateItem;
    IBOutlet NSMenu* runMenu;
    IBOutlet NSMenuItem* runDefaultMenuItem;
    IBOutlet NSMenu* compileMenu;
    IBOutlet NSMenuItem* compileLastMenuItem;
}

- (IBAction)showPreferences:(id)sender;


@end
