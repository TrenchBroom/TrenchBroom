//
//  Face.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Face.h"

@implementation Face

- (id)init {
    if (self = [super init]) {
        point1 = [[Vector3i alloc] init];
        point2 = [[Vector3i alloc] init];
        point3 = [[Vector3i alloc] init];
        texture = [[NSMutableString alloc] init];
    }
    
    return self;
}

- (id) initOnPlane:(Plane)plane at:(Vector3i *)position texture:(NSString *)texture {
    
    Vector3i* p1 = [[Vector3i alloc] initWithVector:position];
    Vector3i* p2 = [[Vector3i alloc] initWithVector:position];
    Vector3i* p3 = [[Vector3i alloc] initWithVector:position];
    
    switch (plane) {
        case XY:
            [p2 addX:1 Y:0 Z:0];
            [p3 addX:0 Y:1 Z:0];
            break;
        case XZ:
            [p2 addX:1 Y:0 Z:0];
            [p3 addX:0 Y:0 Z:1];
            break;
        case YZ:
            [p2 addX:0 Y:1 Z:0];
            [p3 addX:0 Y:0 Z:1];
            break;
    }

    self = [self initWithPoint1:p1 point2:p2 point3:p3 texture:nil];
    
    [p1 release];
    [p2 release];
    [p3 release];
    
    return self;
}

- (id)initWithPoint1:(Vector3i *)aPoint1 point2:(Vector3i *)aPoint2 point3:(Vector3i *)aPoint3 texture:(NSString *)aTexture {
    if (self = [self init]) {
        [self setPoint1:aPoint1];
        [self setPoint2:aPoint2];
        [self setPoint3:aPoint3];
        [self setTexture:aTexture];
    }
    
    return self;
}

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
    [point1 set:point];
}

- (void)setPoint2:(Vector3i *)point {
    [point2 set:point];
}

- (void)setPoint3:(Vector3i *)point {
    [point3 set:point];
}

- (void)setTexture:(NSString *)name {
    [texture setString:name];
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
