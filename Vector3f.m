//
//  Vector3f.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Vector3f.h"
#import "Math3D.h"

@implementation Vector3f

+ (Vector3f *)add:(Vector3f *)left addend:(Vector3f *)right {
    Vector3f* result = [[Vector3f alloc] initWithFloatVector:left];
    [result add:right];
    return [result autorelease];
}

+ (Vector3f *)sub:(Vector3f *)left subtrahend:(Vector3f *)right {
    Vector3f* result = [[Vector3f alloc] initWithFloatVector:left];
    [result sub:right];
    return [result autorelease];
}

+ (Vector3f *)cross:(Vector3f *)left factor:(Vector3f *)right {
    Vector3f* result = [[Vector3f alloc] initWithFloatVector:left];
    [result cross:right];
    return [result autorelease];
}

+ (Vector3f *)normalize:(Vector3f *)vector {
    Vector3f* result = [[Vector3f alloc] initWithFloatVector:vector];
    [result normalize];
    return [result autorelease];
}

- (id)init {
	if (self = [super init]) {
		x = 0;
		y = 0;
		z = 0;
	}
	
	return self;
}

- (id)initWithFloatVector:(Vector3f *)vector {
	if (vector == nil) {
		[self release];
		return nil;
	}
	
	if (self = [super init])
		[self setFloat:vector];
	
	return self;
}

- (id)initWithIntVector:(Vector3i *)vector {
	if (vector == nil) {
		[self release];
		return nil;
	}
	
	if (self = [super init])
		[self setInt:vector];
	
	return self;
}

- (id)initWithX:(float)xCoord y:(float)yCoord z:(float)zCoord {
	if (self = [super init]) {
		[self setX:xCoord];
		[self setY:yCoord];
		[self setZ:zCoord];
	}
	
	return self;
}

- (float)x {
	return x;
}

- (float)y {
	return y;
}

- (float)z {
	return z;
}

- (void)setX:(float)xCoord {
	x = xCoord;
}

- (void)setY:(float)yCoord {
	y = yCoord;
}

- (void)setZ:(float)zCoord {
	z = zCoord;
}

- (void)setFloat:(Vector3f *)vector {
	[self setX:[vector x]];
	[self setY:[vector y]];
	[self setZ:[vector z]];
}

- (void)setInt:(Vector3i *)vector {
	[self setX:[vector x]];
	[self setY:[vector y]];
	[self setZ:[vector z]];
}

- (BOOL)isNull {
    return fabsf(x) <= AlmostZero && fabsf(y) <= AlmostZero && fabsf(z) <= AlmostZero;
}

- (void)add:(Vector3f *)addend {
    [self addX:[addend x] Y:[addend y] Z:[addend z]];
}
- (void)addX:(float)xAddend Y:(float)yAddend Z:(float)zAddend {
    x += xAddend;
    y += yAddend;
    z += zAddend;
}

- (void)sub:(Vector3f *)subtrahend {
    [self subX:[subtrahend x] Y:[subtrahend y] Z:[subtrahend z]];
}

- (void)subX:(float)xSubtrahend Y:(float)ySubtrahend Z:(float)zSubtrahend {
    x -= xSubtrahend;
    y -= ySubtrahend;
    z -= zSubtrahend;
}

- (void)cross:(Vector3f *)m {
    float xt = y * [m z] - z * [m y];
    float yt = z * [m x] - x * [m z];
    z = x * [m y] - y * [m x];
    
    x = xt;
    y = yt;
}

- (float)dot:(Vector3f *)m {
    return x * [m x] + y * [m y] + z * [m z];
}

- (void)normalize {
    float l = [self length];
    x /= l;
    y /= l;
    z /= l;
}

- (float)length {
    return sqrtf([self lengthSquared]);
}

- (float)lengthSquared {
    return x * x + y * y + z * z;
}


- (BOOL)isEqual:(id)object {
    if (object == self)
        return YES;

    if (![object isKindOfClass:[self class]])
        return NO;
    
    Vector3f* vector = (Vector3f*)object;
    return fabsf(x - [vector x]) < AlmostZero && fabsf(y - [vector y]) < AlmostZero && fabsf(z - [vector z]) < AlmostZero;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"X: %f, Y: %f, Z: %f", x, y, z];
}

@end
