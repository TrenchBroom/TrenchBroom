//
//  Camera.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Camera.h"
#import "Vector3f.h"

@implementation Camera

- (id)init {
	
	if (self = [super init]) {
		position = [[Vector3f alloc] initWithXCoord:0 yCoord:0 zCoord:0];
		viewDir = [[Vector3f alloc] initWithXCoord:0 yCoord:0 zCoord:-1];
		up = [[Vector3f alloc] initWithXCoord:0 yCoord:1 zCoord:0];
		right = [[Vector3f alloc] initWithXCoord:1 yCoord:0 zCoord:0];
	}
	
	return self;
}

- (void)dealloc {
	
	[position release];
	[viewDir release];
	[up release];
	[right release];
	
	[super dealloc];
}

@end
