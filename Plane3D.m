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
#import "MathCache.h"

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
        [self setPoint:aPoint norm:aNorm];
    }
    
    return self;
}

- (id)initWithPlane:(Plane3D *)aPlane {
    if (aPlane == nil)
        [NSException raise:NSInvalidArgumentException format:@"plane must not be nil"];
    
    return [self initWithPoint:[aPlane point] norm:[aPlane norm]];
}

- (void)setPoint:(Vector3f *)thePoint norm:(Vector3f *)theNorm {
    if (thePoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    if (theNorm == nil)
        [NSException raise:NSInvalidArgumentException format:@"normal must not be nil"];
    
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
    if (aPoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"aPoint must not be nil"];
    
    MathCache* cache = [MathCache sharedCache];
    Vector3f* t = [cache vector3f];
    [t setFloat:aPoint];
    [t sub:point];
    
    BOOL above = fpos([norm dot:t]);
    [cache returnVector3f:t];
    
    return above;
}


- (Vector3f *)intersectWithLine:(Line3D *)line {
    if (line == nil)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    Vector3f* lp = [line point];
    Vector3f* ld = [line direction];
    float denom = [ld dot:norm];
    if (fzero(denom))
        return nil;

    MathCache* cache = [MathCache sharedCache];
    Vector3f* diff = [cache vector3f];
    [diff setFloat:point];
    [diff sub:lp];

    float d = [diff dot:norm] / denom;
    Vector3f* is = [[Vector3f alloc] initWithFloatVector:ld];
    [is scale:d];
    [is add:lp];
    
    [cache returnVector3f:diff];
    return [is autorelease];
}

- (Vector3f *)intersectWithRay:(Ray3D *)ray {
    if (ray == nil)
        [NSException raise:NSInvalidArgumentException format:@"ray must not be nil"];
    
    Vector3f* ro = [ray origin];
    Vector3f* rd = [ray direction];
    float denom = [rd dot:norm];
    if (fzero(denom))
        return nil;
    
    MathCache* cache = [MathCache sharedCache];
    Vector3f* diff = [cache vector3f];
    [diff setFloat:point];
    [diff sub:ro];

    float d = [diff dot:norm] / denom;
    if (fneg(d))
        return nil;
    
    Vector3f* is = [[Vector3f alloc] initWithFloatVector:rd];
    [is scale:d];
    [is add:ro];
    
    [cache returnVector3f:diff];
    return [is autorelease];
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
