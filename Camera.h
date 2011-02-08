//
//  Camera.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"

extern NSString* const CameraDefaults;
extern NSString* const CameraDefaultsFov;
extern NSString* const CameraDefaultsNear;
extern NSString* const CameraDefaultsFar;

extern NSString* const CameraChanged;

@class Vector3f;
@class Quaternion;

@interface Camera : Observable {
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
@end
