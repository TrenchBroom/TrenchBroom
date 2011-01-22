//
//  Vector3f.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Vector3f.h"
#import "Vector3i.h"
#import "Math.h"

static Vector3f* gXAxisPos;
static Vector3f* gXAxisNeg;
static Vector3f* gYAxisPos;
static Vector3f* gYAxisNeg;
static Vector3f* gZAxisPos;
static Vector3f* gZAxisNeg;

@implementation Vector3f

+ (void)initialize {
    gXAxisPos = [Vector3f vectorWithX:1 y:0 z:0];
    gXAxisNeg = [Vector3f vectorWithX:-1 y:0 z:0];
    gYAxisPos = [Vector3f vectorWithX:0 y:1 z:0];
    gYAxisNeg = [Vector3f vectorWithX:0 y:-1 z:0];
    gZAxisPos = [Vector3f vectorWithX:0 y:0 z:1];
    gZAxisNeg = [Vector3f vectorWithX:0 y:0 z:-1];
}

+ (Vector3f *)vector {
    return [[[Vector3f alloc] init] autorelease];
}

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

+ (Vector3f *)vectorWithFloatVector:(Vector3f *)vector {
    return [[[Vector3f alloc] initWithFloatVector:vector] autorelease];
}

+ (Vector3f *)vectorWithIntVector:(Vector3i *)vector {
    return [[[Vector3f alloc] initWithIntVector:vector] autorelease];
}

+ (Vector3f *)vectorWithX:(float)xCoord y:(float)yCoord z:(float)zCoord {
    return [[[Vector3f alloc] initWithX:xCoord y:yCoord z:zCoord] autorelease];
}

+ (Vector3f *)xAxisPos {
    return gXAxisPos;
}

+ (Vector3f *)xAxisNeg {
    return gXAxisNeg;
}

+ (Vector3f *)yAxisPos {
    return gYAxisPos;
}

+ (Vector3f *)yAxisNeg {
    return gYAxisNeg;
}

+ (Vector3f *)zAxisPos {
    return gZAxisPos;
}

+ (Vector3f *)zAxisNeg {
    return gZAxisNeg;
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

- (float)component:(int)index {
    if (index == 0)
        return x;
    if (index == 1)
        return y;
    if (index == 2)
        return z;
    
    [NSException raise:NSInvalidArgumentException format:@"invalid component index: %i", index];
}

- (void)setComponent:(int)index value:(float)value {
    if (index == 0)
        x = value;
    else if (index == 1) 
        y = value;
    else if (index == 2)
        z = value;
    else
        [NSException raise:NSInvalidArgumentException format:@"invalid component index: %i", index];
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
    return fzero(x) && fzero(y) && fzero(z);
}

- (void)add:(Vector3f *)addend {
    [self addX:[addend x] y:[addend y] z:[addend z]];
}

- (void)addX:(float)xAddend y:(float)yAddend z:(float)zAddend {
    x += xAddend;
    y += yAddend;
    z += zAddend;
}

- (void)sub:(Vector3f *)subtrahend {
    [self subX:[subtrahend x] y:[subtrahend y] z:[subtrahend z]];
}

- (void)subX:(float)xSubtrahend y:(float)ySubtrahend z:(float)zSubtrahend {
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

- (void)scale:(float)f {
    x *= f;
    y *= f;
    z *= f;
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

- (NSComparisonResult)compareToVector:(Vector3f *)vector {
    if (flte(x, [vector x]))
        return NSOrderedAscending;
    if (fgte(x, [vector x]))
        return NSOrderedDescending;
    
    if (flte(y, [vector y]))
        return NSOrderedAscending;
    if (fgte(y, [vector y]))
        return NSOrderedDescending;
        
    if (flte(z, [vector z]))
        return NSOrderedAscending;
    if (fgte(z, [vector z]))
        return NSOrderedDescending;
    
    return NSOrderedSame;
}

- (BOOL)isEqualToVector:(Vector3f *)vector {
    if ([self isEqual:vector])
        return YES;
    
    return feq(x, [vector x]) && feq(y, [vector y]) && feq(z, [vector z]);
}

- (NSString *)description {
    return [NSString stringWithFormat:@"X: %f, Y: %f, Z: %f", x, y, z];
}

@end
