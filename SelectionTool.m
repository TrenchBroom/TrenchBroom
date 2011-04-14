//
//  SelectionTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "SelectionTool.h"
#import "MapWindowController.h"
#import "PickingHitList.h"
#import "PickingHit.h"
#import "SelectionManager.h"
#import "Face.h"
#import "Brush.h"

@implementation SelectionTool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (self = [self init]) {
        windowController = [theWindowController retain];
    }
    
    return self;
}

- (void)handleLeftMouseUp:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {
    SelectionManager* selectionManager = [windowController selectionManager];
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        id <Face> face = [hit object];
        id <Brush> brush = [face brush];
        
        if ([selectionManager mode] == SM_FACES) {
            if ([selectionManager isFaceSelected:face]) {
                [selectionManager addBrush:brush record:NO];
            } else {
                if (([event modifierFlags] & NSCommandKeyMask) == 0) {
                    if ([selectionManager hasSelectedFaces:brush]) {
                        [selectionManager removeAll:NO];
                        [selectionManager addFace:face record:NO];
                    } else {
                        [selectionManager addBrush:brush record:NO];
                    }
                } else {
                    [selectionManager addFace:face record:NO];
                }
            }
        } else {
            if (([event modifierFlags] & NSCommandKeyMask) == 0) {
                if ([selectionManager isBrushSelected:brush]) {
                    [selectionManager addFace:face record:NO];
                } else {
                    [selectionManager removeAll:NO];
                    [selectionManager addBrush:brush record:NO];
                }
            } else {
                if ([selectionManager isBrushSelected:brush]) {
                    [selectionManager removeBrush:brush record:NO];
                } else {
                    [selectionManager addBrush:brush record:NO];
                }
            }
        }
    } else {
        [selectionManager removeAll:NO];
    }
}

- (void)dealloc {
    [windowController release];
    [super dealloc];
}
@end
