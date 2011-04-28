//
//  Vector3i.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Vector3i.h"
#import "Vector3f.h"
#import "Math.h"

static Vector3i* gNullVector;

@implementation Vector3i

+ (void)initialize {
    gNullVector = [[Vector3i alloc] init];
}

+ (Vector3i *)nullVector {
    return gNullVector;
}

- (id)init {
	if (self = [super init]) {
        coords[0] = 0;
        coords[1] = 0;
        coords[2] = 0;
	}
	
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

- (id)initWithFloatVector:(Vector3f *)vector {
    if (self = [super init]) {
        [self setX:roundf([vector x])];
        [self setY:roundf([vector y])];
        [self setZ:roundf([vector z])];
    }
    
    return self;
}

- (id)initWithIntX:(int)xCoord y:(int)yCoord z:(int)zCoord {
	if (self = [super init]) {
		[self setX:xCoord];
		[self setY:yCoord];
		[self setZ:zCoord];
	}
	
	return self;
}

- (int)x {
	return coords[0];
}

- (int)y {
	return coords[1];
}

- (int)z {
	return coords[2];
}

- (void)setX:(int)xCoord {
	coords[0] = xCoord;
}

- (void)setY:(int)yCoord {
	coords[1] = yCoord;
}

- (void)setZ:(int)zCoord {
	coords[2] = zCoord;
}

- (void)setInt:(Vector3i *)vector {
	[self setX:[vector x]];
	[self setY:[vector y]];
	[self setZ:[vector z]];
}

- (void)setFloat:(Vector3f *)vector {
	[self setX:roundf([vector x])];
	[self setY:roundf([vector y])];
	[self setZ:roundf([vector z])];
}

- (void)parse:(NSString *)string {
    int s, l;
    int component = -1;
    char c;

    for (int i = 0; i < [string length] && component != -4; i++) {
        c = [string characterAtIndex:i];
        switch (c) {
            case '-':
            case '+':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                if (component < 0) {
                    s = i;
                    l = 1;
                    component *= -1;
                } else {
                    l++;
                }
                break;
            }
            default:
                if (component > 0) {
                    NSString* numString = [string substringWithRange:NSMakeRange(s, l)];
                    coords[component - 1] = [numString intValue];
                    component++;
                    component *= -1;
                }
                break;
        }
    }
}

- (BOOL)null {
    return coords[0] == 0 && coords[1] == 0 && coords[2] == 0;
}

- (void)add:(Vector3i *)addend {
    coords[0] += [addend x];
    coords[1] += [addend y];
    coords[2] += [addend z];
}

- (void)addX:(int)xAddend Y:(int)yAddend Z:(int)zAddend {
    coords[0] += xAddend;
    coords[1] += yAddend;
    coords[2] += zAddend;
}

- (void)sub:(Vector3i *)subtrahend {
    coords[0] -= [subtrahend x];
    coords[1] -= [subtrahend y];
    coords[2] -= [subtrahend z];
}

- (void)subX:(int)xSubtrahend Y:(int)ySubtrahend Z:(int)zSubtrahend {
    coords[0] -= xSubtrahend;
    coords[1] -= ySubtrahend;
    coords[2] -= zSubtrahend;
}

- (int)dot:(Vector3i *)m {
    return coords[0] * [m x] + coords[1] * [m y] + coords[2] * [m z];
}

- (void)cross:(Vector3i *)m {
    int x = [self y] * [m z] - [self z] * [m y];
    int y = [self z] * [m x] - [self x] * [m z];
    int z = [self x] * [m y] - [self y] * [m x];
    
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
}

- (void)scale:(float)f {
    coords[0] *= f;
    coords[1] *= f;
    coords[2] *= f;
}

- (NSComparisonResult)compareToVector:(Vector3i *)vector {
    if ([self x] < [vector x])
        return NSOrderedAscending;
    if ([self x] > [vector x])
        return NSOrderedDescending;
    
    if ([self y] < [vector y])
        return NSOrderedAscending;
    if ([self y] > [vector y])
        return NSOrderedDescending;
    
    if ([self z] < [vector z])
        return NSOrderedAscending;
    if ([self z] > [vector z])
        return NSOrderedDescending;
    
    return NSOrderedSame;
}


- (BOOL)isEqualToVector:(Vector3i *)vector {
    if ([self isEqual:vector])
        return YES;
    
    return [self x] == [vector x] && [self y] == [vector y] && [self z] == [vector z];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"X: %i, Y: %i, Z: %i", [self x], [self y], [self z]];
}

@end
