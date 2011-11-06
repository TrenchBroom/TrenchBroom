/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    CM_PARALLEL,
    CM_PERSPECTIVE
} ECameraMode;

extern NSString* const CameraChanged;
extern NSString* const CameraViewChanged;

@class EditingSystem;

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
    
    NSMutableSet* animations;
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
- (TVector3f)defaultPointOnRay:(const TRay *)ray;
- (float)distanceTo:(const TVector3f *)thePoint;
- (void)setBillboardMatrix;

- (EditingSystem *)horizontalEditingSystem;
- (EditingSystem *)verticalEditingSystem;

- (NSMutableSet *)animations;

@end
