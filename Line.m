//
//  Line.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Line.h"
#import "Math3D.h";
#import "math.h";

@implementation Line

- (id)initWithPoint:(Vector3f *)point normalizedDirection:(Vector3f *)dir {
	if (!point || !dir) {
		[self release];
		return nil;
	}
	
	if (self = [super init]) {
        p = [[Vector3f alloc] initWithFloatVector:point];
        d = [[Vector3f alloc] initWithFloatVector:dir];
    }
	
	return self;
}

- (id)initWithPoint:(Vector3f *)point direction:(Vector3f *)dir {
	if (!point || !dir) {
		[self release];
		return nil;
	}
	
	if (self = [super init]) {
        p = [[Vector3f alloc] initWithFloatVector:point];
        d = [[Vector3f alloc] initWithFloatVector:dir];
        [d normalize];
    }
	
	return self;
}

- (Vector3f *)p {
    return p;
}

- (Vector3f *)d {
    return d;
}

- (Vector3f *)intersectionWithLine:(Line *)line {
    
    float xp1 = [p x];
    float yp1 = [p y];
    float zp1 = [p z];
    float xd1 = [d x];
    float yd1 = [d y];
    float zd1 = [d z];
    float xp2 = [[line p] x];
    float yp2 = [[line p] y];
    float zp2 = [[line p] z];
    float xd2 = [[line d] x];
    float yd2 = [[line d] y];
    float zd2 = [[line d] z];
    
    float denom = xd1 * yd2 - xd2 * yd1;
    if (fabsf(denom) < AlmostZero)
        return nil;
    
    float t = (xp2 * yd2 - yp2 * xd2 - xp1 * yd2 + xd2 * yp1) / denom;
    
    float x = xp1 + t * xd1;
    float y = yp1 + t * yd1;
    float z = zp1 + t * zd1;
    float sx = (x - xp2) / xd2;
    float sy = (y - yp2) / yd2;
    
    if (fabsf(sx - sy) > AlmostZero)
        return nil;
    
    float sz = (z - zp2) / zd2;
    if (fabsf(sx - sz) > AlmostZero)
        return nil;
    
    return [[[Vector3f alloc] initWithX:x y:y z:z] autorelease];
}

- (LineSide)sideOfPoint:(Vector3f *)point up:(Vector3f *)up {
    if (!point)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    if (!up)
        [NSException raise:NSInvalidArgumentException format:@"up vector must not be nil"];

    // d and up vector form a plane, check on which side of the plane the point is
    // first, get cross of d and up
    Vector3f* c = [Vector3f cross:up factor:d];
    if ([c isNull])
        [NSException raise:NSInvalidArgumentException format:@"up vector and direction of this line must not be colinear"];
    
    Vector3f* r = [Vector3f sub:point subtrahend:p];
    if ([r isNull])
        return LSNeither;
    
    [r normalize];
    float cos = [r dot:c];
    if (cos > AlmostZero)
        return LSLeft;
    if (cos < -AlmostZero)
        return LSRight;
    return LSNeither;
}

- (BOOL)sameDirectionAs:(Line *)l up:(Vector3f *)up {
    if (!l)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    Vector3f* v = [Vector3f cross:d factor:[l d]];
    if ([v isNull])
        return [d isEqualTo:[l d]];
    
    return [v isEqualTo:up];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"point: %@, direction: %@", p, d];
}

- (void)dealloc {
    [p release];
    [d release];
    [super dealloc];
}
@end
