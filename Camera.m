//
//  Camera.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Camera.h"
#import <OpenGL/glu.h>
#import "Vector3f.h"
#import "Quaternion.h"
#import "Ray3D.h"

static NSString* CameraDefaults = @"Camera";
static NSString* CameraDefaultsFov = @"Field Of Vision";
static NSString* CameraDefaultsNear = @"Near Clipping Plane";
static NSString* CameraDefaultsFar = @"Far Clipping Plane";

NSString* const CameraChanged = @"CameraChanged";

@implementation Camera

- (id)init {
    if (self = [super init]) {
        position = [[Vector3f alloc] initWithX:0 y:0 z:0];
        direction = [[Vector3f alloc] initWithX:1 y:0 z:0];
        up = [[Vector3f alloc] initWithX:0 y:0 z:1];
        right = [[Vector3f alloc] initWithFloatVector:direction];
        [right cross:up];
        [right normalize];
        
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

    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)rotateYaw:(float)yaw pitch:(float)pitch {
    Quaternion* qy = [[Quaternion alloc] initWithAngle:yaw axis:up];
    Quaternion* qp = [[Quaternion alloc] initWithAngle:pitch axis:right];
    [qy mul:qp];
    
    Vector3f*  t = [[Vector3f alloc] initWithFloatVector:direction];
    [qy rotate:t];
    
    if (!(([t x] < 0 ^ [direction x] < 0) && ([t y] < 0 ^ [direction y] < 0))) {
        [direction setFloat:t];
        [right setFloat:direction];
        [right cross:up];
        [right normalize];

        [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
    }
    [t release];
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

    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)orbitCenter:(Vector3f *)c hAngle:(float)h vAngle:(float)v {
    Quaternion* qh = [[Quaternion alloc] initWithAngle:h axis:up];
    Quaternion* qv = [[Quaternion alloc] initWithAngle:v axis:right];
    [qh mul:qv];
    
    Vector3f*  t = [[Vector3f alloc] initWithFloatVector:direction];
    [qh rotate:t];
    if (!(([t x] < 0 ^ [direction x] < 0) && ([t y] < 0 ^ [direction y] < 0))) {
        [direction setFloat:t];
        [right setFloat:direction];
        [right cross:up];
        [right normalize];
        
        [position sub:c];
        [qh rotate:position];
        [position add:c];
        
        [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
    }
    [t release];
    [qh release];
    [qv release];
}

- (Vector3f *)unprojectX:(float)x y:(float)y {
    static GLint viewport[4];
    static GLdouble modelview[16];
    static GLdouble projection[16];
    
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    GLdouble rx, ry, rz;
    gluUnProject(x, y, 1, modelview, projection, viewport, &rx, &ry, &rz);
    
    return [[[Vector3f alloc] initWithX:rx y:ry z:rz] autorelease];
}

- (Ray3D *)pickRayX:(float)x y:(float)y {
    Vector3f* rayDir = [self unprojectX:x y:y];
    
    [rayDir sub:position];
    [rayDir normalize];
    
    return [[[Ray3D alloc] initWithOrigin:position direction:rayDir] autorelease];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [position release];
    [direction release];
    [up release];
    [super dealloc];
}

@end
