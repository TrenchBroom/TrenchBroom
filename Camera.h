//
//  Camera.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    CM_PARALLEL,
    CM_PERSPECTIVE
} ECameraMode;

extern NSString* const CameraChanged;
extern NSString* const CameraViewChanged;

@interface Camera : NSObject {
    @private
    TVector3f position;
    TVector3f direction;
    TVector3f up;
    TVector3f right;
    
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

- (const TVector3f *)position;
- (const TVector3f *)direction;
- (const TVector3f *)up;
- (const TVector3f *)right;
- (float)fieldOfVision;
- (float)nearClippingPlane;
- (float)farClippingPlane;
- (float)zoom;
- (ECameraMode)mode;

- (void)moveTo:(const TVector3f *)thePosition;
- (void)moveForward:(float)f right:(float)r up:(float)u;

- (void)lookAt:(const TVector3f *)thePoint up:(const TVector3f *)theUpVector;
- (void)setDirection:(const TVector3f *)theDirection up:(const TVector3f *)theUpVector;

- (void)rotateYaw:(float)yaw pitch:(float)pitch;
- (void)orbitCenter:(const TVector3f *)c hAngle:(float)h vAngle:(float)v;

- (void)setFieldOfVision:(float)theFov;
- (void)setNearClippingPlane:(float)theNear;
- (void)setFarClippingPlane:(float)theFar;
- (void)setZoom:(float)theZoom;
- (void)setMode:(ECameraMode)theMode;

- (void)updateView:(NSRect)theViewport;
- (NSRect)viewport;
- (TVector3f)unprojectX:(float)x y:(float)y depth:(float)depth;
- (TRay)pickRayX:(float)x y:(float)y;
- (TVector3f)defaultPoint;
- (TVector3f)defaultPointAtX:(float)x y:(float)y;
- (float)distanceTo:(TVector3f *)thePoint;
- (void)setBillboardMatrix;
@end
