//
//  Plane.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Plane.h"
#import "Math3D.h"

@implementation Plane

- (id)initWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 {
	if (!point1 || !point2 || !point3) {
		[self release];
		return nil;
	}
	
	if (self = [super init]) {
        Vector3f* p1 = [[Vector3f alloc] initWithIntVector:point1];
        
        Vector3f* p2p1 = [[Vector3f alloc] initWithIntVector:point2];
        [p2p1 sub:p1];
        
        Vector3f* p3p1 = [[Vector3f alloc] initWithIntVector:point3];
        [p3p1 sub:p1];
        
        n = [[Vector3f alloc] initWithFloatVector:p3p1];
        [n cross:p2p1];
        
        [p2p1 release];
        [p3p1 release];
        
        if ([n null]) {
            [p1 release];
            [self release];
            return nil;
        }

        [n normalize];
        d = [n dot:p1];
    }
	
	return self;
}

- (float)d {
    return d;
}

- (Vector3f *)n {
    return n;
}

- (Line3D *)intersectionWithPlane:(Plane *)plane {
    Vector3f *ln = [[Vector3f alloc] initWithFloatVector:n];
    [ln cross:[plane n]];
    
    if ([ln null]) {
        [ln release];
        return nil; // planes are parallel
    }

    Vector3f* lp;
    
    // intersect projections of the planes on the XY plane
    float fx = [n x] / [[plane n] x];
    float dy = [n y] - fx * [[plane n] y];
    if (abs(dy) > AlmostZero) {
        float y = (d - fx * [plane d]) / dy;
        float x = (d - [n y] * y) / [n x];
        float z = (d - [n x] * x - [n y] * y) / [n z];
        lp = [[Vector3f alloc] initWithXCoord:x yCoord:y zCoord:z];
    } else { // try XZ plane
        float dz = [n z] - fx * [[plane n] z];
        if (abs(dz) > AlmostZero) {
            float z = (d - fx * [plane d]) / dz;
            float x = (d - [n z] * z) / [n x];
            float y = (d - [n x] * x - [n z] * z) / [n y];
            lp = [[Vector3f alloc] initWithXCoord:x yCoord:y zCoord:z];
        } else { // try YZ plane
            float fy = [n y] / [[plane n] y];
            dz = [n z] - fy * [[plane n] z];
            float z = (d - fy * [plane d]) / dz;
            float y = (d - [n z] * z) / [n y];
            float x = (d - [n y] * y - [n z] * z) / [n x];
            lp = [[Vector3f alloc] initWithXCoord:x yCoord:y zCoord:z];
        }
    }
    
    Line3D* l = [[Line3D alloc] initWithPoint:lp normalizedDirection:ln];
    [ln release];
    [lp release];
    
    return [l autorelease];
}

- (void)dealloc {
    [n release];
    [super dealloc];
}

@end
