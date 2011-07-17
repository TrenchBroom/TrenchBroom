//
//  EntityModelFrame.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "AliasFrame.h"

@implementation AliasFrame

- (id)initWithName:(NSString *)theName triangles:(TFrameTriangle *)theTriangles triangleCount:(int)theTriangleCount center:(TVector3f *)theCenter bounds:(TBoundingBox *)theBounds maxBounds:(TBoundingBox *)theMaxBounds {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theTriangles != NULL, @"triangle array must not be NULL");
    NSAssert(theTriangleCount > 0, @"triangle count must be greater than 0");
    NSAssert(theCenter != NULL, @"center must not be NULL");
    NSAssert(theBounds != NULL, @"bounds must not be NULL");
    NSAssert(theMaxBounds != NULL, @"max bounds must not be NULL");
    
    if ((self = [self init])) {
        name = [[NSString alloc] initWithString:theName];
        triangleCount = theTriangleCount;
        triangles = malloc(triangleCount * sizeof(TFrameTriangle));
        memcpy(triangles, theTriangles, triangleCount * sizeof(TFrameTriangle));
        center = *theCenter;
        bounds = *theBounds;
        maxBounds = *theMaxBounds;
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    free(triangles);
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (int)triangleCount {
    return triangleCount;
}

- (const TFrameTriangle *)triangleAtIndex:(int)theIndex {
    NSAssert(theIndex >= 0 && theIndex < triangleCount, @"index out of bounds");
    return &triangles[theIndex];
}

- (const TVector3f *)center {
    return &center;
}

- (const TBoundingBox *)bounds {
    return &bounds;
}

- (const TBoundingBox *)maxBounds {
    return &maxBounds;
}

@end
