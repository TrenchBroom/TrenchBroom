//
//  Face.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Face.h"

@implementation Face

- (Vector3i *)point1 {
	return point1;
}

- (Vector3i *)point2 {
	return point2;
}

- (Vector3i *)point3 {
	return point3;
}

- (NSString *)texture {
	return texture;
}

- (int)xOffset {
	return xOffset;
}

- (int)yOffset {
	return yOffset;
}

- (float)rotation {
	return rotation;
}

- (float)xScale {
	return xScale;
}

- (float)yScale {
	return yScale;
}

- (void)setPoint1:(Vector3i *)point{
	[point1 release];
	point1 = [point retain];
}

- (void)setPoint2:(Vector3i *)point {
	[point2 release];
	point2 = [point retain];
}

- (void)setPoint3:(Vector3i *)point {
	[point3 release];
	point3 = [point retain];
}

- (void)setTexture:(NSString *)name {
	[texture release];
	texture = [texture retain];
}

- (void)setXOffset:(int)offset {
	xOffset = offset;
}

- (void)setYOffset:(int)offset {
	yOffset = offset;
}

- (void)setRotation:(float)angle {
	rotation = angle;
}

- (void)setXScale:(float)factor {
	xScale = factor;
}

- (void)setYSCale:(float)factor {
	yScale = factor;
}

- (void) dealloc {
	[point1 release];
	[point2 release];
	[point3 release];
	[texture release];
	
	[super dealloc];
}

@end
