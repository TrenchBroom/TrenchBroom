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
        Vector3f* p = [[Vector3f alloc] initWithIntVector:point1];
        Vector3f* v1 = [[Vector3f alloc] initWithIntVector:point2];
        Vector3f* v2 = [[Vector3f alloc] initWithIntVector:point3];
        [v1 sub:p];
        [v2 sub:p];

        outside = [[Vector3f alloc] initWithFloatVector:v2];
        [outside cross:v1];
        [outside normalize];
        
        boundary = [[Plane3D alloc] initWithPoint:p norm:outside];

        [p release];
        [v1 release];
        [v2 release];
    }
    
    return self;
}

- (BOOL)containsPoint:(Vector3f *)point {
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    
    Vector3f* t = [[Vector3f alloc] initWithFloatVector:point];
    [t sub:[boundary point]];
    
    BOOL contains = flte([t dot:outside], 0); // positive if angle < 90
    [t release];
    
    return contains;
}

- (Plane3D *)boundary {
    return boundary;
}

- (Vector3f *)outside {
    return outside;
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
