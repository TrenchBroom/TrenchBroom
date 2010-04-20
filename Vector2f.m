//
//  Vector2f.m
//  TrenchBroom
//
//  Created by Kristian Duske on 19.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Vector2f.h"
#import "Math3D.h"

@implementation Vector2f
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

- (id)initWithXCoord:(float)xCoord yCoord:(float)yCoord {
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

- (BOOL)null {
    return abs(x) <= AlmostZero && abs(y) <= AlmostZero;
}

- (void)add:(Vector2f *)addend {
    [self addX:[addend x] Y:[addend y]];
}
- (void)addX:(float)xAddend Y:(float)yAddend {
    x += xAddend;
    y += yAddend;
}

- (void)sub:(Vector2f *)subtrahend {
    [self subX:[subtrahend x] Y:[subtrahend y]];
}

- (void)subX:(float)xSubtrahend Y:(float)ySubtrahend {
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
    return abs([self x] - [vector x]) <= AlmostZero && abs([self y] - [vector y]) <= AlmostZero;
}

@end
