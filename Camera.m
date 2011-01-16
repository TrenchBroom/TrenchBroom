//
//  Camera.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Camera.h"
#import "Vector3f.h"

NSString* const CameraDefaults = @"Camera";
NSString* const CameraDefaultsFov = @"FieldOfVision";
NSString* const CameraDefaultsNear = @"NearClippingPlane";
NSString* const CameraDefaultsFar = @"FarClippingPlane";

@implementation Camera

- (id)init {
    if (self = [super init]) {
        position = [[Vector3f alloc] initWithX:0 y:0 z:0];
        direction = [[Vector3f alloc] initWithX:0 y:0 z:-1];
        up = [[Vector3f alloc] initWithX:0 y:1 z:0];

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

- (void)dealloc {
    [position release];
    [direction release];
    [up release];
    [super dealloc];
}

@end
