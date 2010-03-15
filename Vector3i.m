//
//  Vector3i.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Vector3i.h"


@implementation Vector3i

- (id)init {
	if (self = [super init]) {
		x = 0;
		y = 0;
		z = 0;
	}
	
	return self;
}

- (id)initWithVector:(Vector3i *)vector {
	if (vector == nil) {
		[self release];
		return nil;
	}
	
	if (self = [super init])
		[self set:vector];
	
	return self;
}

- (id)initWithXCoord:(int)xCoord yCoord:(int)yCoord zCoord:(int)zCoord {
	if (self = [super init]) {
		[self setX:xCoord];
		[self setY:yCoord];
		[self setZ:zCoord];
	}
	
	return self;
}

- (int)x {
	return x;
}

- (int)y {
	return y;
}

- (int)z {
	return z;
}

- (void)setX:(int)xCoord {
	x = xCoord;
}

- (void)setY:(int)yCoord {
	y = yCoord;
}

- (void)setZ:(int)zCoord {
	z = zCoord;
}

- (void)set:(Vector3i *)vector {
	[self setX:[vector x]];
	[self setY:[vector y]];
	[self setZ:[vector z]];
}

@end
