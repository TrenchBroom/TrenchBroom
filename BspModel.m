//
//  Bsp.m
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BspModel.h"
#import "BspFace.h"

@implementation BspModel

- (id)initWithFaces:(NSArray *)theFaces vertexCount:(int)theVertexCount center:(TVector3f *)theCenter bounds:(TBoundingBox *)theBounds maxBounds:(TBoundingBox *)theMaxBounds {
    NSAssert(theFaces != nil, @"face list must not be nil");
    NSAssert([theFaces count] >= 4, @"face list must contain at least 4 faces");
    NSAssert(theVertexCount >= 12, @"model must have at least 12 vertices");
    NSAssert(theCenter != NULL, @"center must not be NULL");
    NSAssert(theBounds != NULL, @"bounds must not be NULL");
    NSAssert(theMaxBounds != NULL, @"max bounds must not be NULL");
    
    if ((self = [self init])) {
        faces = [theFaces retain];
        vertexCount = theVertexCount;
        center = *theCenter;
        bounds = *theBounds;
        maxBounds = *theMaxBounds;
    }
    
    return self;
}

- (void)dealloc {
    [faces release];
    [super dealloc];
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

- (NSArray *)faces {
    return faces;
}

- (int)vertexCount {
    return vertexCount;
}

@end
