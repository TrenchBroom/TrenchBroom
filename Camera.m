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
		position = [[Vector3f alloc] initWithX:0 y:0 z:0];
		viewDir = [[Vector3f alloc] initWithX:0 y:0 z:-1];
		up = [[Vector3f alloc] initWithX:0 y:1 z:0];
		right = [[Vector3f alloc] initWithX:1 y:0 z:0];
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
