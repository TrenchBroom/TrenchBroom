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

- (Vector3f *)position;
- (Vector3f *)direction;
- (Vector3f *)up;
- (float)fieldOfVision;
- (float)nearClippingPlane;
- (float)farClippingPlane;

- (void)rotateYaw:(float)yaw pitch:(float)pitch;
- (void)moveForward:(float)f right:(float)r up:(float)u;
- (void)orbitCenter:(Vector3f *)c hAngle:(float)h vAngle:(float)v;

- (void)userDefaultsChanged:(NSNotification *)notification;

- (Vector3f *)unprojectX:(float)x y:(float)y;
- (Ray3D *)pickRayX:(float)x y:(float)y;
@end
