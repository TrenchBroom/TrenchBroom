//
//  Line2D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Line2D.h"
#import "Math.h"
#import "Math2D.h"

@implementation Line2D
+ (Line2D *)lineWithPoint1:(Vector2f *)point1 point2:(Vector2f *)point2 {
    return [[[Line2D alloc] initWithPoint1:point1 point2:point2] autorelease];
}

+ (Line2D *)lineWithPoint:(Vector2f *)p normalizedDirection:(Vector2f *)d {
    return [[[Line2D alloc] initWithPoint:p normalizedDirection:d] autorelease];
}

+ (Line2D *)lineWithPoint:(Vector2f *)p direction:(Vector2f *)d {
    return [[[Line2D alloc] initWithPoint:p direction:d] autorelease];
}

+ (Line2D *)lineWithLine:(Line2D *)l {
    return [[[Line2D alloc] initWithLine:l] autorelease];
}

- (id)initWithPoint1:(Vector2f *)point1 point2:(Vector2f *)point2 {
    if (point1 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point1 must not be nil"];
    if (point2 == nil)
        [NSException raise:NSInvalidArgumentException format:@"point2 must not be nil"];
    
    Vector2f* d = [Vector2f sub:point2 subtrahend:point1];
    self = [self initWithPoint:point1 direction:d];
    return self;
}

- (id)initWithPoint:(Vector2f *)p normalizedDirection:(Vector2f *)d {
    if (p == nil)
        [NSException raise:NSInvalidArgumentException format:@"p must not be nil"];
    if (d == nil)
        [NSException raise:NSInvalidArgumentException format:@"d must not be nil"];

	if (self = [super init]) {
        point = [[Vector2f alloc] initWithVector:p];
        direction = [[Vector2f alloc] initWithVector:d];
    }
	
	return self;
}

- (id)initWithPoint:(Vector2f *)p direction:(Vector2f *)d {
	if (p == nil || d == nil) {
		[self release];
		return nil;
	}
	
	if (self = [super init]) {
        point = [[Vector2f alloc] initWithVector:p];
        direction = [[Vector2f alloc] initWithVector:d];
        [direction normalize];
    }
	
	return self;
}

- (id)initWithLine:(Line2D *)l {
    if (l == nil)
        [NSException raise:NSInvalidArgumentException format:@"l must not be nil"];
    return [self initWithPoint:[l point] normalizedDirection:[l direction]];
}

- (Vector2f *)point {
    return point;
}

- (Vector2f *)direction {
    return direction;
}

- (BOOL)isHorizontal {
    return fzero([direction y]);
}

- (BOOL)isVertical {
    return fzero([direction x]);
}

- (float)yAt:(float)x {
    return [direction y] / [direction x] * (x - [point x]) + [point y];
}


- (Vector2f *)intersectWith:(Line2D *)line {
    float xp1 = [point x];
    float yp1 = [point y];
    float xd1 = [direction x];
    float yd1 = [direction y];
    float xp2 = [[line point] x];
    float yp2 = [[line point] y];
    float xd2 = [[line direction] x];
    float yd2 = [[line direction] y];
    
    float denom = xd1 * yd2 - xd2 * yd1;
    if (fzero(denom))
        return nil;
    
    float t = (xp2 * yd2 - yp2 * xd2 - xp1 * yd2 + xd2 * yp1) / denom;
    
    float x = xp1 + t * xd1;
    float y = yp1 + t * yd1;
    
    return [[[Vector2f alloc] initWithX:x y:y] autorelease];
}

- (BOOL)isParallelTo:(Line2D *)line {
    if (line == nil)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];

    float x = [[line point] x];
    float y = [[line point] y];
    
    return [point x] == x && [point y] == y || [point x] == -x && [point y] == -y;
}


- (NSString *)description {
    return [NSString stringWithFormat:@"point: %@, direction: %@", point, direction];
}

- (void)dealloc {
    [point release];
    [direction release];
    [super dealloc];
}
@end
