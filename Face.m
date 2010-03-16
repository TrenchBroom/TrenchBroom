//
//  Face.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Face.h"

NSString* const FacePoint1Changed = @"Point1Changed";
NSString* const FacePoint2Changed = @"Point2Changed";
NSString* const FacePoint3Changed = @"Point3Changed";
NSString* const FaceTextureChanged = @"TextureChanged";
NSString* const FaceXOffsetChanged = @"XOffsetChanged";
NSString* const FaceYOffsetChanged = @"YOffsetChanged";
NSString* const FaceRotationChanged = @"RotationChanged";
NSString* const FaceXScaleChanged = @"XScaleChanged";
NSString* const FaceYScaleChanged = @"YScaleChanged";

NSString* const FacePoint1Old = @"Point1Old";
NSString* const FacePoint1New = @"Point1New";
NSString* const FacePoint2Old = @"Point2Old";
NSString* const FacePoint2New = @"Point2New";
NSString* const FacePoint3Old = @"Point3Old";
NSString* const FacePoint3New = @"Point3New";
NSString* const FaceTextureOld = @"TextureOld";
NSString* const FaceTextureNew = @"TextureNew";
NSString* const FaceXOffsetOld = @"XOffsetOld";
NSString* const FaceXOffsetNew = @"XOffsetNew";
NSString* const FaceYOffsetOld = @"YOffsetOld";
NSString* const FaceYOffsetNew = @"YOffsetNew";
NSString* const FaceRotationOld = @"RotationOld";
NSString* const FaceRotationNew = @"RotationNew";
NSString* const FaceXScaleOld = @"XScaleOld";
NSString* const FaceXScaleNew = @"XScaleNew";
NSString* const FaceYScaleOld = @"YScaleOld";
NSString* const FaceYScaleNew = @"YScaleNew";

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
    if ([point1 isEqual:point])
        return;
    
    Vector3i* old = [[Vector3i alloc] initWithVector:point1];
    [point1 set:point];
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FacePoint1Old];
    [info setObject:point1 forKey:FacePoint1New];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FacePoint1Changed object:self userInfo:info];
}

- (void)setPoint2:(Vector3i *)point {
    if ([point2 isEqual:point])
        return;
    
    Vector3i* old = [[Vector3i alloc] initWithVector:point2];
    [point2 set:point];
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FacePoint2Old];
    [info setObject:point2 forKey:FacePoint2New];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FacePoint2Changed object:self userInfo:info];
}

- (void)setPoint3:(Vector3i *)point {
    if ([point3 isEqual:point])
        return;
    
    Vector3i* old = [[Vector3i alloc] initWithVector:point3];
    [point3 set:point];

    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FacePoint3Old];
    [info setObject:point3 forKey:FacePoint3New];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FacePoint3Changed object:self userInfo:info];
}

- (void)setTexture:(NSString *)name {
    if ([texture isEqualToString:name])
        return;
    
    NSString* old = [[NSString alloc] initWithString:texture];
    [texture setString:name];
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FaceTextureOld];
    [info setObject:texture forKey:FaceTextureNew];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FaceTextureChanged object:self userInfo:info];
}

- (void)setXOffset:(int)offset {
    if (xOffset == offset)
        return;
    
    NSNumber* old = [[NSNumber alloc] initWithInt:xOffset];
    NSNumber* new = [[NSNumber alloc] initWithInt:offset];
    
	xOffset = offset;
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FaceXOffsetOld];
    [info setObject:new forKey:FaceXOffsetNew];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FaceXOffsetChanged object:self userInfo:info];
}

- (void)setYOffset:(int)offset {
    if (yOffset == offset)
        return;
    
    NSNumber* old = [[NSNumber alloc] initWithInt:yOffset];
    NSNumber* new = [[NSNumber alloc] initWithInt:offset];
    
	yOffset = offset;
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FaceYOffsetOld];
    [info setObject:new forKey:FaceYOffsetNew];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FaceYOffsetChanged object:self userInfo:info];
}

- (void)setRotation:(float)angle {
    if (rotation == angle)
        return;
    
    NSNumber* old = [[NSNumber alloc] initWithFloat:rotation];
    NSNumber* new = [[NSNumber alloc] initWithFloat:angle];
    
	rotation = angle;
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FaceRotationOld];
    [info setObject:new forKey:FaceRotationNew];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FaceRotationChanged object:self userInfo:info];
}

- (void)setXScale:(float)factor {
    if (xScale == factor)
        return;
    
    NSNumber* old = [[NSNumber alloc] initWithFloat:xScale];
    NSNumber* new = [[NSNumber alloc] initWithFloat:factor];
    
	xScale = factor;
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FaceXScaleOld];
    [info setObject:new forKey:FaceXScaleNew];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FaceXScaleChanged object:self userInfo:info];
}

- (void)setYSCale:(float)factor {
    if (yScale == factor)
        return;
    
    NSNumber* old = [[NSNumber alloc] initWithFloat:yScale];
    NSNumber* new = [[NSNumber alloc] initWithFloat:factor];
    
	yScale = factor;
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:old forKey:FaceYScaleOld];
    [info setObject:new forKey:FaceYScaleNew];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:FaceYScaleChanged object:self userInfo:info];
}

- (void) dealloc {
	[point1 release];
	[point2 release];
	[point3 release];
	[texture release];
	
	[super dealloc];
}

@end
