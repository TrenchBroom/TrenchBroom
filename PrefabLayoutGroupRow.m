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
        titleBarBounds = NSMakeRect(thePos.x, thePos.y, theWidth, nameSize.height + 4);
        titleBounds = NSMakeRect(thePos.x + 4, thePos.y + 2, nameSize.width, nameSize.height);

        float x = thePos.x;
        float y = NSMaxY(titleBarBounds) + innerMargin;
        float cellWidth = theWidth / thePrefabsPerRow - (thePrefabsPerRow - 1) * innerMargin;
        PrefabLayoutPrefabCell* cell;
        
        NSArray* prefabs = [prefabGroup prefabs];
        for (int i = 0; i < [prefabs count]; i++) {
            if (i > 0 && i % thePrefabsPerRow == 0) {
                x = thePos.x;
                y += cellWidth + innerMargin;
            }

            id <Prefab> prefab = [prefabs objectAtIndex:i];
            cell = [[PrefabLayoutPrefabCell alloc] initWithPrefab:prefab glFont:theGLFont atPos:NSMakePoint(x, y) width:cellWidth];
            [cells addObject:cell];
            [cell release];
            
            x += cellWidth + innerMargin;
        }

        float height = NSMaxY([cell bounds]) - thePos.y;
        
        bounds = NSMakeRect(thePos.x, thePos.y, theWidth, height);
    }
    
    return self;
}

- (id <PrefabGroup>)prefabGroup {
    return prefabGroup;
}

- (NSArray *)cells {
    return cells;
}

- (id <Prefab>)prefabAt:(NSPoint)pos {
    NSEnumerator* cellEn = [cells objectEnumerator];
    PrefabLayoutPrefabCell* cell;
    while ((cell = [cellEn nextObject]))
        if (NSPointInRect(pos, [cell prefabBounds]))
            return [cell prefab];
    return nil;
}

- (NSRect)titleBarBounds {
    return titleBarBounds;
}

- (NSRect)titleBounds {
    return titleBounds;
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
