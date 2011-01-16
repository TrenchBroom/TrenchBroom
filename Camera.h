//
//  Camera.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const CameraDefaults;
extern NSString* const CameraDefaultsFov;
extern NSString* const CameraDefaultsNear;
extern NSString* const CameraDefaultsFar;

@class Vector3f;

@interface Camera : NSObject {
    @private
    Vector3f* position;
    Vector3f* direction;
    Vector3f* up;
    
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

- (void)userDefaultsChanged:(NSNotification *)notification;

@end
