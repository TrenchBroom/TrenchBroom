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

- (id)initWithIntPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 {
    if (self = [super init]) {
        point = [[Vector3f alloc] initWithIntVector:point1];
        Vector3f* v1 = [[Vector3f alloc] initWithIntVector:point2];
        Vector3f* v2 = [[Vector3f alloc] initWithIntVector:point3];
        [v1 sub:point];
        [v2 sub:point];
        
        norm = [[Vector3f alloc] initWithFloatVector:v2];
        [norm cross:v1];
        [norm normalize];
        
        [v1 release];
        [v2 release];
    }
    
    return self;
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

- (EPointStatus)pointStatus:(Vector3f *)thePoint {
    Vector3f* t = [[Vector3f alloc] initWithFloatVector:thePoint];
    [t sub:point];
    
    EPointStatus status;
    if (fpos([norm dot:t]))
        status = PS_ABOVE;
    else if (fneg([norm dot:t]))
        status = PS_BELOW;
    else
        status = PS_INSIDE;
    [t release];
    
    return status;
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

- (float)xAtY:(float)y z:(float)z {
    float l = [norm dot:point];
    return (l - [norm y] * y - [norm z] * z) / [norm x];
}

- (float)yAtX:(float)x z:(float)z{
    float l = [norm dot:point];
    return (l - [norm x] * x - [norm z] * z) / [norm y];
}

- (float)zAtX:(float)x y:(float)y {
    float l = [norm dot:point];
    return (l - [norm x] * x - [norm y] * y) / [norm z];
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
