//
//  Line.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Line3D.h"
#import "Math3D.h";

@implementation Line3D

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

- (Vector3f *)intersectionWithLine:(Line3D *)line {
    
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
    
    float t = (xp2 * yd2 - yp2 * xd2 - xp1 * yd2 + xd2 * yp1) / (xd1 * yd2 - xd2 * yd1);

    float x = xp1 + t * xd1;
    float y = yp1 + t * yd1;
    float z = zp1 + t * zd1;
    float sx = (x - xp2) / xd2;
    float sy = (y - yp2) / yd2;
    
    if (abs(sx - sy) > AlmostZero)
        return nil;
    
    float sz = (z - zp2) / zd2;
    if (abs(sx - sz) > AlmostZero)
        return nil;
    
    Vector3f* v = [[Vector3f alloc] initWithX:x Y:y Z:z];
    return [v autorelease];
}

- (void)dealloc {
    [p release];
    [d release];
    [super dealloc];
}
@end
