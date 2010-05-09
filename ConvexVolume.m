//
//  ConvexVolume.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ConvexVolume.h"

@implementation ConvexVolume

- (id)init {
    if (self == [super init]) {
        planes = [[NSMutableSet alloc] init];
        vertices = [[NSMutableSet alloc] init];
        edgesPerPlane = [[NSMutableDictionary alloc] init];
        verticesPerPlane = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)addPlane:(Plane *)plane {
    if ([planes count] == 0) {
        [planes addObject:plane];
        return;
    }

}

- (void)dealloc {
    [verticesPerPlane release];
    [edgesPerPlane release];
    [vertices release];
    [planes release];
    [super dealloc];
}

@end
