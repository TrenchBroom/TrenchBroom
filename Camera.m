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
#import "MathCache.h"

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
    }
    
    return self;
}

- (id)initWithFieldOfVision:(float)theFov nearClippingPlane:(float)theNear farClippingPlane:(float)theFar {
    if (self = [self init]) {
        fov = theFov;
        near = theNear;
        far = theFar;
    }
    
    return self;
}

- (id)initWithCamera:(Camera *)theCamera {
    if (self = [self init]) {
        [position setFloat:[theCamera position]];
        [direction setFloat:[theCamera direction]];
        [up setFloat:[theCamera up]];
        
        [right setFloat:direction];
        [right cross:up];
        [right normalize];

        fov = [theCamera fieldOfVision];
        near = [theCamera nearClippingPlane];
        far = [theCamera farClippingPlane];
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

- (void)moveTo:(Vector3f *)thePosition {
    [position setFloat:thePosition];
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)lookAt:(Vector3f *)thePoint {
    [direction setFloat:thePoint];
    [direction sub:position];
    [direction normalize];

    [right setFloat:direction];
    [right cross:up];
    [right normalize];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)setDirection:(Vector3f *)theDirection {
    [direction setFloat:theDirection];
    [direction normalize];
    
    [right setFloat:direction];
    [right cross:up];
    [right normalize];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)rotateYaw:(float)yaw pitch:(float)pitch {
    MathCache* cache = [MathCache sharedCache];
    
    Quaternion* qy = [cache quaternion];
    Quaternion* qp = [cache quaternion];
    [qy setAngle:yaw axis:up];
    [qp setAngle:pitch axis:right];
    [qy mul:qp];
    
    Vector3f*  t = [cache vector3f];
    [t setFloat:direction];
    [qy rotate:t];
    
    if (!(([t x] < 0 ^ [direction x] < 0) && ([t y] < 0 ^ [direction y] < 0))) {
        [direction setFloat:t];
        [right setFloat:direction];
        [right cross:up];
        [right normalize];

        [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
    }
    
    [cache returnVector3f:t];
    [cache returnQuaternion:qy];
    [cache returnQuaternion:qp];
}

- (void)moveForward:(float)f right:(float)r up:(float)u {
    MathCache* cache = [MathCache sharedCache];
    
    Vector3f* v = [cache vector3f];
    [v setFloat:direction];
    [v scale:f];
    [position add:v];
    
    [v setFloat:right];
    [v scale:r];
    [position add:v];
    
    [v setFloat:up];
    [v scale:u];
    [position add:v];

    [cache returnVector3f:v];

    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)orbitCenter:(Vector3f *)c hAngle:(float)h vAngle:(float)v {
    MathCache* cache = [MathCache sharedCache];
    
    Quaternion* qh = [cache quaternion];
    Quaternion* qv = [cache quaternion];
    [qh setAngle:v axis:right];
    [qv setAngle:h axis:up];
    [qh mul:qv];
    
    Vector3f*  t = [cache vector3f];
    [t setFloat:direction];
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
    
    [cache returnVector3f:t];
    [cache returnQuaternion:qh];
    [cache returnQuaternion:qv];
}

- (void)setFieldOfVision:(float)theFov {
    fov = theFov;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)setNearClippingPlane:(float)theNear {
    near = theNear;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)setFarCliippingPlane:(float)theFar {
    far = theFar;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)updateView:(NSRect)bounds {
    glViewport(NSMinX(bounds), NSMinY(bounds), NSWidth(bounds), NSHeight(bounds));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, NSWidth(bounds) / NSHeight(bounds), near, far);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt([position x],
              [position y],
              [position z],
              [position x] + [direction x],
              [position y] + [direction y],
              [position z] + [direction z],
              [up x],
              [up y],
              [up z]);
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
