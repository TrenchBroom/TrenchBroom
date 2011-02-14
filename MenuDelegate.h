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
}

@end
