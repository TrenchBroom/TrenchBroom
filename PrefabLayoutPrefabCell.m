//
//  PrefabLayoutPrefabCell.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PrefabLayoutPrefabCell.h"
#import "Prefab.h"
#import "GLFont.h"


@implementation PrefabLayoutPrefabCell

- (id)initWithPrefab:(id <Prefab>)thePrefab glFont:(GLFont *)theGLFont atPos:(NSPoint)thePos width:(float)theWidth {
    if (self = [self init]) {
        prefab = [thePrefab retain];
        prefabBounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth);

        NSSize nameSize = [theGLFont sizeOfString:[prefab name]];
        nameBounds = NSMakeRect(thePos.x + (theWidth - nameSize.width) / 2, NSMaxY(prefabBounds), nameSize.width, nameSize.height);
        bounds = NSMakeRect(thePos.x, thePos.y, theWidth, theWidth + nameSize.height);
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
