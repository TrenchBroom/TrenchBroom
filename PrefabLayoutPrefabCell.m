//
//  PrefabLayoutPrefabCell.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PrefabLayoutPrefabCell.h"
#import "Prefab.h"

@implementation PrefabLayoutPrefabCell

- (id)initWithPrefab:(id <Prefab>)thePrefab atPos:(NSPoint)thePos width:(float)theWidth nameSize:(NSSize)theNameSize {
    if ((self = [self init])) {
        prefab = [thePrefab retain];
        prefabBounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth);

        nameBounds = NSMakeRect(thePos.x + (theWidth - theNameSize.width) / 2, NSMaxY(prefabBounds), theNameSize.width, theNameSize.height);
        bounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth + theNameSize.height);
    }
    
    return self;
}

- (id <Prefab>)prefab {
    return prefab;
}

- (NSRect)prefabBounds {
    return prefabBounds;
}

- (NSRect)nameBounds {
    return nameBounds;
}

- (NSRect)bounds {
    return bounds;
}

- (void)dealloc {
    [prefab release];
    [super dealloc];
}

@end
