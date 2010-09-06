//
//  Vector2f.m
//  2fTrenchBroom
//
//  Created by Kristian Duske on 19.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Vector2f.h"
#import "Math.h"

@implementation Vector2f
+ (Vector2f *)add:(Vector2f *)left addend:(Vector2f *)right {
    Vector2f* result = [[Vector2f alloc] initWithVector:left];
    [result add:right];
    return [result autorelease];
}

+ (Vector2f *)sub:(Vector2f *)left subtrahend:(Vector2f *)right {
    Vector2f* result = [[Vector2f alloc] initWithVector:left];
    [result sub:right];
    return [result autorelease];
}

+ (Vector2f *)normalize:(Vector2f *)vector {
    Vector2f* result = [[Vector2f alloc] initWithVector:vector];
    [result normalize];
    return [result autorelease];
}

- (id)init {
	if (self = [super init]) {
		x = 0;
		y = 0;
	}
	
	return self;
}

- (id)initWithVector:(Vector2f *)vector {
	if (vector == nil) {
		[self release];
		return nil;
	}
	
	if (self = [super init])
		[self set:vector];
	
	return self;
}

- (id)initWithX:(float)xCoord y:(float)yCoord {
	if (self = [super init]) {
		[self setX:xCoord];
		[self setY:yCoord];
	}
	
	return self;
}

- (float)x {
	return x;
}

- (float)y {
	return y;
}

- (void)setX:(float)xCoord {
	x = xCoord;
}

- (void)setY:(float)yCoord {
	y = yCoord;
}

- (void)set:(Vector2f *)vector {
	[self setX:[vector x]];
	[self setY:[vector y]];
}

- (BOOL)isNull {
    return abs(x) <= AlmostZero && abs(y) <= AlmostZero;
}

- (void)add:(Vector2f *)addend {
    [self addX:[addend x] y:[addend y]];
}
- (void)addX:(float)xAddend y:(float)yAddend {
    x += xAddend;
    y += yAddend;
}

- (void)sub:(Vector2f *)subtrahend {
    [self subX:[subtrahend x] y:[subtrahend y]];
}

- (void)subX:(float)xSubtrahend y:(float)ySubtrahend {
    x -= xSubtrahend;
    y -= ySubtrahend;
}

- (float)dot:(Vector2f *)m {
    return x * [m x] + y * [m y];
}

- (void)normalize {
    float l = [self length];
    x /= l;
    y /= l;
}

- (float)length {
    return sqrtf([self lengthSquared]);
}

- (float)lengthSquared {
    return x * x + y * y;
}


- (BOOL)isEqual:(id)object {
    if (object == self)
        return YES;
    
    if (![object isKindOfClass:[self class]])
        return NO;
    
    Vector2f* vector = (Vector2f*)object;
    return fabsf([self x] - [vector x]) <= AlmostZero && fabsf([self y] - [vector y]) <= AlmostZero;
}

- (NSComparisonResult)lexicographicCompare:(Vector2f *)vector {
    if (x < [vector x] - AlmostZero)
        return NSOrderedAscending;
    if (x > [vector x] + AlmostZero)
        return NSOrderedDescending;
    if (y < [vector y] - AlmostZero)
        return NSOrderedAscending;
    if (y > [vector y] + AlmostZero)
        return NSOrderedDescending;
    return NSOrderedSame;
}

- (BOOL)isSmallerThan:(Vector2f *)vector {
    return [self lexicographicCompare:vector] == NSOrderedAscending;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"X: %f, Y: %f", x, y];
}

@end
