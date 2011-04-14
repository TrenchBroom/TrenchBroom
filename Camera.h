//
//  Camera.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    CM_PARALLEL,
    CM_PERSPECTIVE
} ECameraMode;

extern NSString* const CameraChanged;
extern NSString* const CameraViewChanged;

@class Vector3f;
@class Quaternion;
@class Ray3D;

@interface Camera : NSObject {
    @private
    Vector3f* position;
    Vector3f* direction;
    Vector3f* up;
    Vector3f* right;
    
    NSRect viewport;
    float fov;
    float near;
    float far;
    ECameraMode mode;
    float zoom;
    
    GLdouble modelview[16];
    GLdouble projection[16];
}

- (id)initWithFieldOfVision:(float)theFov nearClippingPlane:(float)theNear farClippingPlane:(float)theFar;
- (id)initWithCamera:(Camera *)theCamera;

- (Vector3f *)position;
- (Vector3f *)direction;
- (Vector3f *)up;
- (Vector3f *)right;
- (float)fieldOfVision;
- (float)nearClippingPlane;
- (float)farClippingPlane;
- (float)zoom;
- (ECameraMode)mode;

- (void)moveTo:(Vector3f *)thePosition;
- (void)moveForward:(float)f right:(float)r up:(float)u;

- (void)lookAt:(Vector3f *)thePoint up:(Vector3f *)theUpVector;
- (void)setDirection:(Vector3f *)theDirection up:(Vector3f *)theUpVector;

- (void)rotateYaw:(float)yaw pitch:(float)pitch;
- (void)orbitCenter:(Vector3f *)c hAngle:(float)h vAngle:(float)v;

- (void)setFieldOfVision:(float)theFov;
- (void)setNearClippingPlane:(float)theNear;
- (void)setFarClippingPlane:(float)theFar;
- (void)setZoom:(float)theZoom;
- (void)setMode:(ECameraMode)theMode;

- (void)updateView:(NSRect)theViewport;
- (NSRect)viewport;
- (Vector3f *)unprojectX:(float)x y:(float)y;
- (Ray3D *)pickRayX:(float)x y:(float)y;
- (Vector3f *)defaultPoint;
@end
