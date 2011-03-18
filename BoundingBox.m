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
    [self mergeMin:[theBounds min] max:[theBounds max]];
}

- (void)mergeMin:(Vector3f *)theMin max:(Vector3f *)theMax {
    if ([theMin x] < [min x])
        [min setX:[theMin x]];
    if ([theMin y] < [min y])
        [min setY:[theMin y]];
    if ([theMin z] < [min z])
        [min setZ:[theMin z]];
    if ([theMax x] > [max x])
        [max setX:[theMax x]];
    if ([theMax y] > [max y])
        [max setY:[theMax y]];
    if ([theMax z] > [max z])
        [max setZ:[theMax z]];
    
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
