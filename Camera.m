//
//  Camera.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Camera.h"
#import "Vector3f.h"
#import "Quaternion.h"

NSString* const CameraDefaults = @"Camera";
NSString* const CameraDefaultsFov = @"FieldOfVision";
NSString* const CameraDefaultsNear = @"NearClippingPlane";
NSString* const CameraDefaultsFar = @"FarClippingPlane";

@implementation Camera

- (id)init {
    if (self = [super init]) {
        position = [[Vector3f alloc] initWithX:256 y:0 z:0];
        direction = [[Vector3f alloc] initWithX:-1 y:0 z:0];
        up = [[Vector3f alloc] initWithX:0 y:0 z:1];
        right = [[Vector3f alloc] initWithFloatVector:direction];
        [right cross:up];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self 
                   selector:@selector(userDefaultsChanged:) 
                       name:NSUserDefaultsDidChangeNotification 
                     object:[NSUserDefaults standardUserDefaults]];
        
        [self userDefaultsChanged:nil];
    }
    
    return self;
}

- (Vector3f *)position {
    return position;
}

- (Vector3f *)direction {
    return direction;
}

- (Vector3f *)up {
    return up;
}

- (float)fieldOfVision {
    return fov;
}

- (float)nearClippingPlane {
    return near;
}

- (float)farClippingPlane {
    return far;
}

- (void)userDefaultsChanged:(NSNotification *)notification {
    NSDictionary* cameraDefaults = [[NSUserDefaults standardUserDefaults] dictionaryForKey:CameraDefaults];
    if (cameraDefaults == nil)
        return;
    
    fov = [[cameraDefaults objectForKey:CameraDefaultsFov] floatValue];
    near = [[cameraDefaults objectForKey:CameraDefaultsNear] floatValue];
    far = [[cameraDefaults objectForKey:CameraDefaultsFar] floatValue];
}

- (void)rotateYaw:(float)yaw pitch:(float)pitch {
    Quaternion* qy = [[Quaternion alloc] initWithAngle:yaw axis:up];
    Quaternion* qp = [[Quaternion alloc] initWithAngle:pitch axis:right];
    [qy mul:qp];
    
    [qy rotate:direction];
    [right setFloat:direction];
    [right cross:up];
    
    [qy release];
    [qp release];
}

- (void)moveForward:(float)f right:(float)r up:(float)u {
    Vector3f* v = [[Vector3f alloc] initWithFloatVector:direction];
    [v scale:f];
    [position add:v];
    
    [v setFloat:right];
    [v scale:r];
    [position add:v];
    
    [v setFloat:up];
    [v scale:u];
    [position add:v];
    
    [v release];
}

- (void)dealloc {
    [position release];
    [direction release];
    [up release];
    [super dealloc];
}

@end
