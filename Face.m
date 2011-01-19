//
//  Face.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Face.h"
#import "IdGenerator.h"
#import "Vector3i.h"
#import "HalfSpace3D.h"
#import "Vector3f.h"

@implementation Face

- (id)init {
    if (self = [super init]) {
        faceId = [[IdGenerator sharedGenerator] getId];
        point1 = [[Vector3i alloc] init];
        point2 = [[Vector3i alloc] init];
        point3 = [[Vector3i alloc] init];
        texture = [[NSMutableString alloc] init];
    }
    
    return self;
}

- (id) initOnPlane:(EPlaneType)plane at:(Vector3i *)position thirdAxisPositive:(BOOL)thirdAxisPositive texture:(NSString *)aTexture {
    Vector3i* p1 = [[Vector3i alloc] initWithVector:position];
    Vector3i* p2 = [[Vector3i alloc] initWithVector:position];
    Vector3i* p3 = [[Vector3i alloc] initWithVector:position];
    
    switch (plane) {
        case XY:
            if (thirdAxisPositive) {
                [p2 addX:0 Y:1 Z:0];
                [p3 addX:1 Y:0 Z:0];
            } else {
                [p2 addX:1 Y:0 Z:0];
                [p3 addX:0 Y:1 Z:0];
            }
            break;
        case XZ:
            if (thirdAxisPositive) {
                [p2 addX:1 Y:0 Z:0];
                [p3 addX:0 Y:0 Z:1];
            } else {
                [p2 addX:0 Y:0 Z:1];
                [p3 addX:1 Y:0 Z:0];
            }
            break;
        case YZ:
            if (thirdAxisPositive) {
                [p2 addX:0 Y:0 Z:1];
                [p3 addX:0 Y:1 Z:0];
            } else {
                [p2 addX:0 Y:1 Z:0];
                [p3 addX:0 Y:0 Z:1];
            }
            break;
    }

    self = [self initWithPoint1:p1 point2:p2 point3:p3 texture:aTexture];
    
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

- (NSNumber *)getId {
    return faceId;
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
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];

    [point1 set:point];
}

- (void)setPoint2:(Vector3i *)point {
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];

    [point2 set:point];
}

- (void)setPoint3:(Vector3i *)point {
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];

    [point3 set:point];
}

- (void)setTexture:(NSString *)name {
    if (name == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture name must not be nil"];
    
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

- (void)setYScale:(float)factor {
	yScale = factor;
}

- (HalfSpace3D *)halfSpace {
    return [HalfSpace3D halfSpaceWithIntPoint1:[self point1] 
                                        point2:[self point2] 
                                        point3:[self point3]];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"ID: @i, point 1: %@, point 2: %@, point 3: %@, texture: %@, X offset: %i, Y offset: %i, rotation: %f, X scale: %f, Y scale: %f", 
            [faceId intValue], 
            point1, 
            point2, 
            point3, 
            texture, 
            xOffset, 
            yOffset, 
            rotation, 
            xScale, 
            yScale];
}

- (void) dealloc {
	[point1 release];
	[point2 release];
	[point3 release];
	[texture release];
	
	[super dealloc];
}

@end
