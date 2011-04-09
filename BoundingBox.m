//
//  BoundingBox.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "BoundingBox.h"
#import "Vector3f.h"
#import "Brush.h"

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

- (id)initWithBrushes:(NSSet *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    NSAssert([theBrushes count] > 0, @"brush set must not be empty");
    
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    id <Brush> brush = [brushEn nextObject];
    
    if (self = [self initWithBounds:[brush bounds]]) {
        while ((brush = [brushEn nextObject]))
            [self mergeBounds:[brush bounds]];
    }
    
    return self;
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

- (Vector3f *)center {
    Vector3f* center = [[Vector3f alloc] initWithFloatVector:size];
    [center scale:0.5f];
    [center add:min];
    return [center autorelease];
}

- (void)mergeBounds:(BoundingBox *)theBounds {
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

- (void)mergePoint:(Vector3f *)thePoint {
    if ([thePoint x] < [min x])
        [min setX:[thePoint x]];
    else if ([thePoint x] > [max x])
        [max setX:[thePoint x]];
    if ([thePoint y] < [min y])
        [min setY:[thePoint y]];
    else if ([thePoint y] > [max y])
        [max setY:[thePoint y]];
    if ([thePoint z] < [min z])
        [min setZ:[thePoint z]];
    else if ([thePoint z] > [max z])
        [max setZ:[thePoint z]];
    
    [size setFloat:max];
    [size sub:min];
}

- (void)expandBy:(float)delta {
    [min subX:delta y:delta z:delta];
    [max addX:delta y:delta z:delta];
    [size addX:2 * delta y:2 * delta z:2 * delta];
}

- (void)dealloc {
    [min release];
    [max release];
    [size release];
    [super dealloc];
}

@end
