//
//  Camera.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const CameraChanged;

@class Vector3f;
@class Quaternion;
@class Ray3D;

@interface Camera : NSObject {
    @private
    Vector3f* position;
    Vector3f* direction;
    Vector3f* up;
    Vector3f* right;
    
    float fov;
    float near;
    float far;
}

- (id)initWithFieldOfVision:(float)theFov nearClippingPlane:(float)theNear farClippingPlane:(float)theFar;
- (id)initWithCamera:(Camera *)theCamera;

- (Vector3f *)position;
- (Vector3f *)direction;
- (Vector3f *)up;
- (float)fieldOfVision;
- (float)nearClippingPlane;
- (float)farClippingPlane;

- (void)moveTo:(Vector3f *)thePosition;
- (void)lookAt:(Vector3f *)thePoint;
- (void)setDirection:(Vector3f *)theDirection;
- (void)rotateYaw:(float)yaw pitch:(float)pitch;
- (void)moveForward:(float)f right:(float)r up:(float)u;
- (void)orbitCenter:(Vector3f *)c hAngle:(float)h vAngle:(float)v;
- (void)setFieldOfVision:(float)theFov;
- (void)setNearClippingPlane:(float)theNear;
- (void)setFarCliippingPlane:(float)theFar;
- (void)updateView:(NSRect)bounds;

- (Vector3f *)unprojectX:(float)x y:(float)y;
- (Ray3D *)pickRayX:(float)x y:(float)y;
@end
