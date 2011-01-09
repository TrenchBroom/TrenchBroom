//
//  Plane3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Plane3D.h"
#import "Vector3f.h"
#import "Line3D.h"
#import "Math.h"

@implementation Plane3D
+ (Plane3D *)planeWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm {
    return [[[Plane3D alloc] initWithPoint:aPoint norm:aNorm] autorelease];
}

+ (Plane3D *)planeWithPlane:(Plane3D *)aPlane {
    return [[[Plane3D alloc] initWithPlane:aPlane] autorelease];
}

- (id)initWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm {
    if (aPoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    if (aNorm == nil)
        [NSException raise:NSInvalidArgumentException format:@"normal must not be nil"];
    
    if (self == [super init]) {
        point = [[Vector3f alloc] initWithFloatVector:aPoint];
        norm = [[Vector3f alloc] initWithFloatVector:aNorm];
    }
    
    return self;
}

- (id)initWithPlane:(Plane3D *)aPlane {
    if (aPlane == nil)
        [NSException raise:NSInvalidArgumentException format:@"plane must not be nil"];
    
    return [self initWithPoint:[aPlane point] norm:[aPlane norm]];
}

- (Vector3f *)point {
    return point;
}

- (Vector3f *)norm {
    return norm;
}

- (BOOL)isPointAbove:(Vector3f *)aPoint {
    if (aPoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"aPoint must not be nil"];
    
    Vector3f* t = [Vector3f sub:aPoint subtrahend:point];
    return fpos([norm dot:t]);
}


- (Vector3f *)intersectWithLine:(Line3D *)line {
    if (line == nil)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    Vector3f* lp = [line point];
    Vector3f* ld = [line direction];
    float denom = [ld dot:norm];
    if (fzero(denom))
        return nil;

    Vector3f* diff = [Vector3f sub:point subtrahend:lp];
    float d = [diff dot:norm] / denom;
    
    Vector3f* is = [[Vector3f alloc] initWithFloatVector:ld];
    [is scale:d];
    [is add:lp];
    
    return [is autorelease];
}

- (Segment3D *)intersectWithPolygon:(Polygon3D *)polygon {
    NSArray* vertices = [polygon vertices];
    Vector3f* prevVert = [vertices lastObject];
    BOOL prevAbove = [self isPointAbove:prevVert];
    Vector3f* curVert;
    BOOL curAbove;
    
    Vector3f* start;
    Vector3f* end;
    
    for (int i = 0; i < [vertices count]; i++) {
        curVert = [vertices objectAtIndex:i];
        curAbove = [self isPointAbove:curVert];
        
        if (prevAbove ^ curAbove) {
            Line3D* line = [[Line3D alloc] initWithPoint1:prevVert point2:curVert];
            if (start == nil)
                start = [self intersectWithLine:line];
            else
                end = [self intersectWithLine:line];
            [line release];
        }
        
        prevVert = curVert;
        prevAbove = curAbove;
    }
    
    if (start == nil || end == nil)
        return nil;
    
    return [Segment3D segmentWithStartVertex:start endVertex:end];
}

- (void)dealloc {
    [point release];
    [norm release];
    [super dealloc];
}

@end
