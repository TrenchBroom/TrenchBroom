//
//  Polygon3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon3D.h"
#import "Math3D.h"
#import "Plane3D.h"

@implementation Polygon3D
- (id)init {
    if (self == [super init])
        vertices = [[NSMutableArray alloc] init];

    return self;
}

- (id)initWithVertices:(NSArray *)someVertices {
    if (someVertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
    if ([someVertices count] < 3)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must contain at least three vertices"];
        
        
    if (self = [super init]) {
        int si = smallestVertex(someVertices);
        if (si == 0) {
            vertices = [[NSMutableArray alloc] initWithArray:someVertices];
        } else {
            int c = [someVertices count];
            vertices = [[NSMutableArray alloc] initWithCapacity:c];
            int i;
            for (i = 0; i < c; i++)
                [vertices addObject:[someVertices objectAtIndex:(i + si) % c]];
        }
    }
    
    return self;
}

- (BOOL)intersectWith:(HalfSpace3D *)halfSpace {
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
            Vector3f* newVert = [boundary intersectWith:line];
            [newVertices addObject:newVert];
            [line release];
        }
        
        prevVert = curVert;
        prevContained = curContained;
    }
    
    [vertices release];
    vertices = newVertices;
    
    return [vertices count] != 0;
}

- (BOOL)isEqual:(id)object {
    return NO;
}

- (void)dealloc {
    [vertices release];
    [super dealloc];
}
@end
