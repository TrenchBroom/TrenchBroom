//
//  Bsp.m
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BspModel.h"


@implementation BspModel

- (id)initWithFaces:(NSArray *)theFaces vertexCount:(int)theVertexCount {
    NSAssert(theFaces != nil, @"face list must not be nil");
    NSAssert([theFaces count] >= 4, @"face list must contain at least 4 faces");
    NSAssert(theVertexCount >= 12, @"model must have at least 12 vertices");
    
    if ((self = [self init])) {
        faces = [theFaces retain];
        vertexCount = theVertexCount;
    }
    
    return self;
}

- (void)dealloc {
    [faces release];
    [super dealloc];
}

- (NSArray *)faces {
    return faces;
}

- (int)vertexCount {
    return vertexCount;
}

@end
