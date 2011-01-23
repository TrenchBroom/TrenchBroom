//
//  Polygon3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Polygon3D.h"
#import "HalfSpace3D.h"
#import "Plane3D.h"
#import "Line3D.h"
#import "Math3D.h"
#import "Vector3f.h"
#import "Segment3D.h"

@implementation Polygon3D
+ (Polygon3D *)polygonWithVertices:(NSArray *)someVertices norm:(Vector3f *)aNorm {
    return [[[Polygon3D alloc] initWithVertices:someVertices norm:aNorm] autorelease];
}

+ (Polygon3D *)polygonWithVertices:(NSArray *)someVertices {
    return [[[Polygon3D alloc] initWithVertices:someVertices] autorelease];
}

- (id)init {
    if (self == [super init]) {
        vertices = [[NSMutableArray alloc] init];
        norm = [[Vector3f alloc] init];
    }
    
    return self;
}

- (id)initWithVertices:(NSArray *)someVertices norm:(Vector3f *)aNorm {
    if (someVertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
    if ([someVertices count] < 3)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must contain at least three vertices"];
    if (aNorm == nil)
        [NSException raise:NSInvalidArgumentException format:@"normal must not be nil"];
        
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
        
        norm = [[Vector3f alloc] initWithFloatVector:aNorm];
    }
    
    return self;
}

- (id)initWithVertices:(NSArray *)someVertices {
    if (someVertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
    if ([someVertices count] < 3)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must contain at least three vertices"];

    // assumes CCW order of vertices
    Vector3f* v1 = [Vector3f sub:[someVertices objectAtIndex:1] subtrahend:[someVertices objectAtIndex:0]];
    Vector3f* v2 = [Vector3f sub:[someVertices objectAtIndex:2] subtrahend:[someVertices objectAtIndex:1]];
    [v2 cross:v1];
    [v2 normalize];
    
    return [self initWithVertices:someVertices norm:v2];
}

- (NSArray *)vertices {
    return vertices;
}

- (Vector3f *)norm {
    return norm;
}

- (BOOL)isEqualToPolygon:(Polygon3D *)polygon {
    if ([self isEqual:polygon])
        return YES;
    
    Vector3f* otherNorm = [polygon norm];
    if (![otherNorm isEqualToVector:norm])
        return NO;
    
    NSArray* otherVertices = [polygon vertices];
    if ([vertices count] != [otherVertices count])
        return NO;
    
    for (int i = 0; i < [vertices count]; i++) {
        Vector3f* vertex = [vertices objectAtIndex:i];
        Vector3f* otherVertex = [otherVertices objectAtIndex:i];
        
        if (![vertex isEqualToVector:otherVertex])
            return NO;
    }
    
    return YES;
}

- (BOOL)intersectWithHalfSpace:(HalfSpace3D *)halfSpace newSegment:(Segment3D **)newSegment {
    Vector3f* newSegmentStart = nil;
    Vector3f* newSegmentEnd = nil;
    
    Vector3f* prevVert = [vertices lastObject];
    BOOL prevContained = [halfSpace containsPoint:prevVert];
    Vector3f* curVert;
    BOOL curContained;
    BOOL outside = !prevContained;
    
    NSMutableArray* newVertices = [[NSMutableArray alloc] init];
    NSEnumerator* vertEn = [vertices objectEnumerator];
    while ((curVert = [vertEn nextObject])) {
        curContained = [halfSpace containsPoint:curVert];
        
        if (prevContained)
            [newVertices addObject:prevVert];
        
        if (prevContained ^ curContained) {
            Line3D* line = [[Line3D alloc] initWithPoint1:prevVert point2:curVert];
            Vector3f* newVert = [[halfSpace boundary] intersectWithLine:line];
            [newVertices addObject:newVert];
            [line release];
            
            if (newSegmentStart == nil)
                newSegmentStart = newVert;
            else if (newSegmentEnd == nil)
                newSegmentEnd = newVert;
        }
        
        prevVert = curVert;
        prevContained = curContained;
        outside &= !prevContained;
    }
    
    [vertices release];
    vertices = newVertices;
    
    if (outside)
        return NO;
    
    if (newSegmentStart != nil && newSegmentEnd != nil)
        *newSegment = [Segment3D segmentWithStartVertex:newSegmentStart endVertex:newSegmentEnd];
    
    return YES;
}

- (NSString *)description {
    NSMutableString* descr = [NSMutableString stringWithFormat:@"normal: %@, vertices: ", norm];
    NSEnumerator* vertexEn = [vertices objectEnumerator];
    Vector3f* vertex;
    while ((vertex = [vertexEn nextObject]))
        [descr appendFormat:@"\n  %@", [vertex description]];
    return descr;
}

- (void)dealloc {
    [norm release];
    [vertices release];
    [super dealloc];
}
@end
