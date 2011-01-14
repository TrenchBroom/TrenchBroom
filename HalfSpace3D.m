//
//  HalfSpace3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "HalfSpace3D.h"
#import "Vector3f.h"
#import "Vector3i.h"
#import "Plane3D.h"
#import "Line3D.h"
#import "Polygon3D.h"
#import "Segment3D.h"
#import "Polyhedron.h"
#import "Math.h"


@implementation HalfSpace3D
+ (HalfSpace3D *)halfSpaceWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside {
    return [[[HalfSpace3D alloc] initWithBoundary:theBoundary outside:theOutside] autorelease];
}

+ (HalfSpace3D *)halfSpaceWithIntPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 {
    return [[[HalfSpace3D alloc] initWithIntPoint1:point1 point2:point2 point3:point3] autorelease];
}

- (id)initWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside {
    if (theBoundary == nil)
        [NSException raise:NSInvalidArgumentException format:@"boundary must not be nil"];
    if (theOutside == nil)
        [NSException raise:NSInvalidArgumentException format:@"outside must not be nil"];
    if ([theOutside isNull])
        [NSException raise:NSInvalidArgumentException format:@"outside must not be null"];
    
    if (self = [super init]) {
        boundary = [[Plane3D alloc] initWithPlane:theBoundary];
        outside = [[Vector3f alloc] initWithFloatVector:theOutside];
    }
    
    return self;
}

- (id)initWithIntPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 {
    if (point1 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point1 must not be nil"];
    if (point2 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point2 must not be nil"];
    if (point3 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point3 must not be nil"];
    
    if (self = [super init]) {
        Vector3f* p = [Vector3f vectorWithIntVector:point1];
        Vector3f* v1 = [Vector3f vectorWithIntVector:point2];
        Vector3f* v2 = [Vector3f vectorWithIntVector:point3];
        [v1 sub:p];
        [v2 sub:p];

        outside = [[Vector3f cross:v2 factor:v1] retain];
        [outside normalize];
        
        boundary = [[Plane3D alloc] initWithPoint:v1 norm:outside];
    }
    
    return self;
}

- (BOOL)containsPoint:(Vector3f *)point {
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    
    Vector3f* t = [Vector3f sub:point subtrahend:[boundary point]];
    return flte([t dot:outside], 0); // positive if angle < 90
}

- (BOOL)containsPolygon:(Polygon3D *)polygon {
    if (polygon == nil)
        [NSException raise:NSInvalidArgumentException format:@"polygon must not be nil"];
    
    NSEnumerator* vertexEnum = [[polygon vertices] objectEnumerator];
    Vector3f* vertex;
    
    while ((vertex = [vertexEnum nextObject]))
        if (![self containsPoint:vertex])
            return NO;

    return YES;
}

- (BOOL)containsPolyhedron:(Polyhedron *)polyhedron {
    if (polyhedron == nil)
        [NSException raise:NSInvalidArgumentException format:@"polyhedron must not be nil"];
    
    NSEnumerator* sideEnum = [[polyhedron sides] objectEnumerator];
    Polygon3D* polygon;
    
    while ((polygon = [sideEnum nextObject]))
        if (![self containsPolygon:polygon])
            return NO;
    
    return YES;
}

- (Segment3D *)intersectWithPolygon:(Polygon3D *)polygon vertexArray:(NSMutableArray *)newVertices {
    NSArray* vertices = [polygon vertices];
    Vector3f* newSegmentStart = nil;
    Vector3f* newSegmentEnd = nil;

    Vector3f* prevVert = [vertices lastObject];
    BOOL prevContained = [self containsPoint:prevVert];
    Vector3f* curVert;
    BOOL curContained;
    
    BOOL contained = prevContained;
    
    for (int i = 0; i < [vertices count]; i++) {
        curVert = [vertices objectAtIndex:i];
        curContained = [self containsPoint:curVert];
        
        if (prevContained)
            [newVertices addObject:prevVert];
        
        if (prevContained ^ curContained) {
            Line3D* line = [[Line3D alloc] initWithPoint1:prevVert point2:curVert];
            Vector3f* newVert = [boundary intersectWithLine:line];
            [newVertices addObject:newVert];
            [line release];
            
            if (newSegmentStart == nil)
                newSegmentStart = newVert;
            else if (newSegmentEnd == nil)
                newSegmentEnd = newVert;
        }
        
        prevVert = curVert;
        prevContained = curContained;
        contained &= prevContained;
    }
    
    if (newSegmentStart == nil || newSegmentEnd == nil)
        return nil;
    
    return [Segment3D segmentWithStartVertex:newSegmentStart endVertex:newSegmentEnd];
}

- (Polygon3D *)intersectWithPolygon:(Polygon3D *)polygon {
    if (polygon == nil)
        [NSException raise:NSInvalidArgumentException format:@"polygon must not be nil"];

    NSMutableArray* newVertices = [NSMutableArray array];
    [self intersectWithPolygon:polygon vertexArray:newVertices];

    if ([newVertices count] == 0)
        return nil;
    
    return [Polygon3D polygonWithVertices:newVertices];
}

- (Polyhedron *)intersectWithPolyhedron:(Polyhedron *)polyhedron {
    if (polyhedron == nil)
        [NSException raise:NSInvalidArgumentException format:@"polyhedron must not be nil"];
    
    NSEnumerator* sideEn = [[polyhedron sides] objectEnumerator];
    NSMutableSet* newSides = [NSMutableSet set];
    NSMutableArray* newVertices = [NSMutableArray array];
    NSMutableArray* newSegments = [NSMutableArray array];

    Polygon3D* side;
    while ((side = [sideEn nextObject])) {
        Segment3D* newSegment = [self intersectWithPolygon:side vertexArray:newVertices];
        if ([newVertices count] != 0)
            [newSides addObject:[Polygon3D polygonWithVertices:newVertices]];
        if (newSegment != nil)
            [newSegments addObject:newSegment];
        [newVertices removeAllObjects];
    }
    
    // sort the new segments and add their start vertices in order to newVerts
    for (int i = 0; i < [newSegments count]; i++) {
        Segment3D* segment = [newSegments objectAtIndex:i];
        [newVertices addObject:[segment startVertex]];
        for (int j = i + 2; j < [newSegments count]; j++) {
            Segment3D* candidate = [newSegments objectAtIndex:j];
            if ([[segment endVertex] isEqualToVector:[candidate startVertex]]) {
                [newSegments exchangeObjectAtIndex:i + 1 withObjectAtIndex:j];
                break;
            }
        }
    }
    
    [newSides addObject:[Polygon3D polygonWithVertices:newVertices]];
    return [Polyhedron polyhedronWithSides:newSides];
    return nil;
}

- (Plane3D *)boundary {
    return boundary;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"boundary: %@, outside: %@", boundary, outside];
}

- (void)dealloc {
    [boundary release];
    [outside release];
    [super dealloc];
}
@end
