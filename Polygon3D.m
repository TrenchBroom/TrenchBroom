//
//  Polygon3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon3D.h"

@implementation Polygon3D
- (id)init {
    if (self == [super init])
        vertices = [[NSMutableArray alloc] init];

    return self;
}

- (id)initWithVertices:(NSArray *)someVertices {
    if (someVertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
        
    if (self = [super init])
        vertices = [[NSMutableArray alloc] initWithArray:someVertices];
    
    return self;
}

- (void)intersectWith:(HalfSpace3D *)halfSpace {
    NSMutableArray* newVertices = [[NSMutableArray alloc] init];
    Vector3f* prevVert = [vertices lastObject];
    BOOL prevContained = [halfSpace contains:prevVert];
    Vector3f* curVert;
    BOOL curContained;
    
    for (int i = 0; i < [vertices count]; i++) {
        curVert = [vertices objectAtIndex:i];
        curContained = [halfSpace contains:curVert];
        
        if (prevContained)
            [newVertices addObject:prevVert];
        
        if (prevContained ^ curContained) {
            Plane3D* boundary = [halfSpace boundary];
            Line3D* line = [[Line3D alloc] initWithPoint1:prevVert point2:curVert];
            [newVertices addObject:[boundary intersectWith:line]];
            [line release];
        }
        
        prevVert = curVert;
        prevContained = curContained;
    }
    
    [vertices release];
    vertices = newVertices;
}

- (void)dealloc {
    [vertices release];
    [super dealloc];
}
@end
