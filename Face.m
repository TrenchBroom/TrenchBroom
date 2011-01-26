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
#import "Vector2f.h"
#import "math.h"

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

- (id)initWithPoint1:(Vector3i *)aPoint1 point2:(Vector3i *)aPoint2 point3:(Vector3i *)aPoint3 texture:(NSString *)aTexture {
    if (self = [self init]) {
        [self setPoint1:aPoint1];
        [self setPoint2:aPoint2];
        [self setPoint3:aPoint3];
        [self setTexture:aTexture];
        [self setXScale:1];
        [self setYScale:1];
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

- (Vector3f *)norm {
    if (norm == nil) {
        Vector3f* p1f = [[Vector3f alloc] initWithIntVector:point1];
        Vector3f* v1 = [[Vector3f alloc] initWithIntVector:point3];
        Vector3f* v2 = [[Vector3f alloc]initWithIntVector:point2];
        
        [v1 sub:p1f];
        [v2 sub:p1f];
        [v1 cross:v2];
        
        [v1 normalize];
        norm = v1;
        [v2 release];
        [p1f release];
    }
    
    return norm;
}

- (void)setPoint1:(Vector3i *)point{
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];

    [point1 set:point];
    [norm release];
    norm = nil;
}

- (void)setPoint2:(Vector3i *)point {
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];

    [point2 set:point];
    [norm release];
    norm = nil;
}

- (void)setPoint3:(Vector3i *)point {
    if (point == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];

    [point3 set:point];
    [norm release];
    norm = nil;
}

- (void)setTexture:(NSString *)name {
    if (name == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture name must not be nil"];
    
    [texture setString:name];
}

- (void)setXOffset:(int)offset {
	xOffset = offset;

    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
}

- (void)setYOffset:(int)offset {
	yOffset = offset;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
}

- (void)setRotation:(float)angle {
	rotation = angle;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
}

- (void)setXScale:(float)factor {
	xScale = factor;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
}

- (void)setYScale:(float)factor {
	yScale = factor;
    
    [texAxisX release];
    texAxisX = nil;
    [texAxisY release];
    texAxisY = nil;
}

- (HalfSpace3D *)halfSpace {
    return [HalfSpace3D halfSpaceWithIntPoint1:[self point1] 
                                        point2:[self point2] 
                                        point3:[self point3]];
}

- (void)texCoords:(Vector2f *)texCoords forVertex:(Vector3f *)vertex {
    if (texCoords == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture coordinate vector must not be nil"];
    if (vertex == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex must not be nil"];
    
    if (texAxisX == nil || texAxisY == nil) {
        // determine texture axes, this is from QBSP
        Vector3f* baseAxes[18] = {
            [Vector3f zAxisPos], [Vector3f xAxisPos], [Vector3f yAxisNeg],
            [Vector3f zAxisNeg], [Vector3f xAxisPos], [Vector3f yAxisNeg],
            [Vector3f xAxisPos], [Vector3f yAxisPos], [Vector3f zAxisNeg],
            [Vector3f xAxisNeg], [Vector3f yAxisPos], [Vector3f zAxisNeg],
            [Vector3f yAxisPos], [Vector3f xAxisPos], [Vector3f zAxisNeg],
            [Vector3f yAxisNeg], [Vector3f xAxisPos], [Vector3f zAxisNeg]
        };
        
        float best = 0;
        int bestAxis = 0;
        for (int i = 0; i < 6; i++) {
            float dot = [[self norm] dot:baseAxes[i * 3]];
            if (dot > best) {
                best = dot;
                bestAxis = i;
            }
        }
        
        texAxisX = [[Vector3f alloc] initWithFloatVector:baseAxes[bestAxis * 3 + 1]];
        texAxisY = [[Vector3f alloc] initWithFloatVector:baseAxes[bestAxis * 3 + 2]];
        
        float ang = rotation / 180 * M_PI;
        float sinv = sin(ang);
        float cosv = cos(ang);
        
        int sv, tv;
        if ([texAxisX x] != 0)
            sv = 0;
        else if ([texAxisX y] != 0)
            sv = 1;
        else
            sv = 2;
            
        if ([texAxisY x] != 0)
            tv = 0;
        else if ([texAxisY y] != 0)
            tv = 1;
        else
            tv = 2;
        
        Vector3f* texAxes[2] = {texAxisX, texAxisY};
        for (int i = 0; i < 2; i++) {
            float ns = cosv * [texAxes[i] component:sv] - sinv * [texAxes[i] component:tv];
            float nt = sinv * [texAxes[i] component:sv] + cosv * [texAxes[i] component:tv];
            
            [texAxes[i] setComponent:sv value:ns];
            [texAxes[i] setComponent:tv value:nt];
        }
        
        [texAxisX scale:1 / xScale];
        [texAxisY scale:1 / yScale];
    }
    
    [texCoords setX:[vertex dot:texAxisX] + xOffset];
    [texCoords setY:[vertex dot:texAxisY] + yOffset];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"ID: %i, point 1: %@, point 2: %@, point 3: %@, texture: %@, X offset: %i, Y offset: %i, rotation: %f, X scale: %f, Y scale: %f", 
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
    [norm release];
    [texAxisX release];
    [texAxisY release];
	
	[super dealloc];
}

@end
