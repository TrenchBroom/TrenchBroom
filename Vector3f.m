//
//  Vector3f.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Vector3f.h"


@implementation Vector3f

- (id)init {
	if (self = [super init]) {
		x = 0;
		y = 0;
		z = 0;
	}
	
	return self;
}

- (id)initWithVector:(Vector3f *)vector {
	if (vector == nil) {
		[self release];
		return nil;
	}
	
	if (self = [super init])
		[self set:vector];
	
	return self;
}

- (id)initWithXCoord:(float)xCoord yCoord:(float)yCoord zCoord:(float)zCoord {
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

- (void)set:(Vector3f *)vector {
	[self setX:[vector x]];
	[self setY:[vector y]];
	[self setZ:[vector z]];
}

@end
