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
#import "Ray3D.h"
#import "Math.h"

@implementation Plane3D
+ (Plane3D *)planeWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm {
    return [[[Plane3D alloc] initWithPoint:aPoint norm:aNorm] autorelease];
}

+ (Plane3D *)planeWithPlane:(Plane3D *)aPlane {
    return [[[Plane3D alloc] initWithPlane:aPlane] autorelease];
}

- (id)initWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm {
    if (self == [super init]) {
        [self setPoint:aPoint norm:aNorm];
    }
    
    return self;
}

- (id)initWithPlane:(Plane3D *)aPlane {
    return [self initWithPoint:[aPlane point] norm:[aPlane norm]];
}

- (void)setPoint:(Vector3f *)thePoint norm:(Vector3f *)theNorm {
    [point release];
    point = [thePoint retain];
    
    [norm release];
    norm = [theNorm retain];
}

- (Vector3f *)point {
    return point;
}

- (Vector3f *)norm {
    return norm;
}

- (BOOL)isPointAbove:(Vector3f *)aPoint {
    Vector3f* t = [[Vector3f alloc] initWithFloatVector:aPoint];;
    [t sub:point];
    
    BOOL above = fpos([norm dot:t]);
    [t release];
    
    return above;
}

- (BOOL)containsPoint:(Vector3f *)aPoint {
    Vector3f* t = [[Vector3f alloc] initWithFloatVector:aPoint];;
    [t sub:point];
    
    BOOL contains = fzero([norm dot:t]);
    [t release];
    
    return contains;
}

- (float)intersectWithLine:(Line3D *)line {
    Vector3f* lp = [line point];
    Vector3f* ld = [line direction];
    float denom = [ld dot:norm];
    if (fzero(denom))
        return NAN;

    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:point];
    [diff sub:lp];
    float dist = [diff dot:norm] / denom;

    [diff release];
    return dist;
}

- (float)intersectWithRay:(Ray3D *)ray {
    Vector3f* ro = [ray origin];
    Vector3f* rd = [ray direction];
    float denom = [rd dot:norm];
    if (fzero(denom))
        return NAN;
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:point];
    [diff sub:ro];
    float d = [diff dot:norm] / denom;
    [diff release];

    if (fneg(d))
        return NAN;
    
    return d;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"point: %@, normal: %@", point, norm];
}

- (void)dealloc {
    [point release];
    [norm release];
    [super dealloc];
}

@end
