//
//  BoundingBox.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "BoundingBox.h"
#import "Vector3f.h"

@implementation BoundingBox
- (id)initWithMin:(Vector3f *)theMin max:(Vector3f *)theMax {
    if (self = [self init]) {
        min = [[Vector3f alloc] initWithFloatVector:theMin];
        max = [[Vector3f alloc] initWithFloatVector:theMax];
        size = [[Vector3f alloc] initWithFloatVector:max];
        [size sub:min];
    }
    
    return self;
}

- (id)initWithBounds:(BoundingBox *)theBounds {
    return [self initWithMin:[theBounds min] max:[theBounds max]];
}

- (Vector3f *)min {
    return min;
}

- (Vector3f *)max {
    return max;
}

- (Vector3f *)size {
    return size;
}

- (void)merge:(BoundingBox *)theBounds {
    Vector3f* otherMin = [theBounds min];
    Vector3f* otherMax = [theBounds max];
    if ([otherMin x] < [min x])
        [min setX:[otherMin x]];
    if ([otherMin y] < [min y])
        [min setY:[otherMin y]];
    if ([otherMin z] < [min z])
        [min setZ:[otherMin z]];
    if ([otherMax x] > [max x])
        [max setX:[otherMax x]];
    if ([otherMax y] > [max y])
        [max setY:[otherMax y]];
    if ([otherMax z] > [max z])
        [max setZ:[otherMax z]];
    
    [size setFloat:max];
    [size sub:min];
}

- (void)dealloc {
    [min release];
    [max release];
    [size release];
    [super dealloc];
}

@end
