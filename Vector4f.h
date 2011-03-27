//
//  Vector4f.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface Vector4f : NSObject {
@private
	float x;
	float y;
	float z;
    float w;
}

- (id)initWithVector3f:(Vector3f *)vector;
- (id)initWithVector4f:(Vector4f *)vector;
- (id)initWithFloatX:(float)xCoord y:(float)yCoord z:(float)zCoord w:(float)wCoord;

- (float)x;
- (float)y;
- (float)z;
- (float)w;

- (float)component:(int)index;
- (void)setComponent:(int)index value:(float)value;

- (void)setX:(float)xCoord;
- (void)setY:(float)yCoord;
- (void)setZ:(float)zCoord;
- (void)setW:(float)wCoord;

- (void)setVector3f:(Vector3f *)vector;
- (void)setVector4f:(Vector4f *)vector;

- (void)getVector3f:(Vector3f *)vector;

- (BOOL)isNull;

- (void)scale:(float)f;
- (void)normalize;
- (float)length;
- (float)lengthSquared;

- (BOOL)isEqualToVector:(Vector4f *)vector;

@end
