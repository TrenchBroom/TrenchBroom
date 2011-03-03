//
//  PrefabLayoutGroupRow.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PrefabLayoutGroupRow.h"
#import "PrefabLayoutPrefabCell.h"
#import "PrefabGroup.h"
#import "Prefab.h"
#import "GLFont.h"

@implementation PrefabLayoutGroupRow

- (id)init {
    if (self = [super init]) {
        cells = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithPrefabGroup:(id <PrefabGroup>)thePrefabGroup prefabsPerRow:(int)thePrefabsPerRow glFont:(GLFont *)theGLFont atPos:(NSPoint)thePos width:(float)theWidth innerMargin:(float)innerMargin {
    if (self = [self init]) {
        prefabGroup = [thePrefabGroup retain];
        NSSize nameSize = [theGLFont sizeOfString:[prefabGroup name]];
        nameBounds = NSMakeRect(thePos.x, thePos.y, nameSize.width, nameSize.height);

        float x = thePos.x;
        float y = thePos.y + nameSize.height + innerMargin;
        float cellWidth = theWidth / thePrefabsPerRow - (thePrefabsPerRow - 1) * innerMargin;
        
        NSArray* prefabs = [prefabGroup prefabs];
        for (int i = 0; i < [prefabs count]; i++) {
            if (i > 0 && i % thePrefabsPerRow == 0) {
                x = thePos.x;
                y += cellWidth + innerMargin;
            }

            id <Prefab> prefab = [prefabs objectAtIndex:i];
            PrefabLayoutPrefabCell* cell = [[PrefabLayoutPrefabCell alloc] initWithPrefab:prefab glFont:theGLFont atPos:NSMakePoint(x, y) width:cellWidth];
            [cells addObject:cell];
            [cell release];
            
            x += cellWidth + innerMargin;
        }
        
        bounds = NSMakeRect(thePos.x, thePos.y, theWidth, y + cellWidth + innerMargin);
    }
    
    return self;
}

- (id <PrefabGroup>)prefabGroup {
    return prefabGroup;
}

- (NSArray *)cells {
    return cells;
}

- (NSRect)nameBounds {
    return nameBounds;
}

- (NSRect)bounds {
    return bounds;
}

- (void)dealloc {
    [prefabGroup release];
    [cells release];
    [super dealloc];
}

@end
