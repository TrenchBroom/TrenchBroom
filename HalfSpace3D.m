//
//  HalfSpace3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "HalfSpace3D.h"


@implementation HalfSpace3D
+ (HalfSpace3D *)halfSpaceWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside {
    return [[[HalfSpace3D alloc] initWithBoundary:theBoundary outside:theOutside] autorelease];
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

- (BOOL)contains:(Vector3f *)point {
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    
    Vector3f* t = [Vector3f sub:point subtrahend:[boundary point]];
    return flte([t dot:outside], 0); // positive if angle < 90
}

- (Plane3D *)boundary {
    return boundary;
}

- (void)dealloc {
    [boundary release];
    [outside release];
    [super dealloc];
}
@end
