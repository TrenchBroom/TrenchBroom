//
//  Line3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 09.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Line3D.h"
#import "Vector3f.h"

@implementation Line3D
+ (Line3D *)lineWithPoint1:(Vector3f *)point1 point2:(Vector3f *)point2 {
    return [[[Line3D alloc] initWithPoint1:point1 point2:point2] autorelease];
}

+ (Line3D *)lineWithPoint:(Vector3f *)aPoint normalizedDirection:(Vector3f *)aDirection {
    return [[[Line3D alloc] initWithPoint:aPoint normalizedDirection:aDirection] autorelease];
}

+ (Line3D *)lineWithPoint:(Vector3f *)aPoint direction:(Vector3f *)aDirection {
    return [[[Line3D alloc] initWithPoint:aPoint direction:aDirection] autorelease];
}

+ (Line3D *)lineWithLine:(Line3D *)aLine {
    return [[[Line3D alloc] initWithLine:aLine] autorelease];
}

- (id)init {
    if (self = [super init]) {
        point = [[Vector3f alloc] init];
        direction = [[Vector3f alloc] initWithFloatVector:[Vector3f zAxisPos]];
    }
    
    return self;
}

- (id)initWithPoint1:(Vector3f *)point1 point2:(Vector3f *)point2 {
    if (point1 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point1 must not be nil"];
    if (point2 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point2 must not be nil"];
    
    Vector3f* d = [Vector3f sub:point2 subtrahend:point1];
    self = [self initWithPoint:point1 direction:d];
    return self;
}

- (id)initWithPoint:(Vector3f *)aPoint normalizedDirection:(Vector3f *)aDirection {
    if (aPoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    if (aDirection == nil)
        [NSException raise:NSInvalidArgumentException format:@"direction must not be nil"];
    
	if (self = [super init]) {
        point = [[Vector3f alloc] initWithFloatVector:aPoint];
        direction = [[Vector3f alloc] initWithFloatVector:aDirection];
    }
	
	return self;
}

- (id)initWithPoint:(Vector3f *)aPoint direction:(Vector3f *)aDirection {
    if (aPoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    if (aDirection == nil)
        [NSException raise:NSInvalidArgumentException format:@"direction must not be nil"];

	if (self = [super init]) {
        point = [[Vector3f alloc] initWithFloatVector:aPoint];
        direction = [[Vector3f alloc] initWithFloatVector:aDirection];
        [direction normalize];
    }
	
	return self;
}

- (id)initWithLine:(Line3D *)aLine {
    if (aLine == nil)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    self = [self initWithPoint:[aLine point] normalizedDirection:[aLine direction]];
    return self;
}

- (void)setPoint1:(Vector3f *)thePoint1 point2:(Vector3f *)thePoint2 {
    [point setFloat:thePoint1];
    [direction setFloat:thePoint2];
    
    [direction sub:point];
    [direction normalize];
}

- (Vector3f *)point {
    return point;
}

- (Vector3f *)direction {
    return direction;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"point: %@, direction: %@", point, direction];
}

- (void) dealloc {
    [point release];
    [direction release];
    [super dealloc];
}

@end
