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
        position = [[Vector3f alloc] initWithFloatVector:[Vector3f nullVector]];
        direction = [[Vector3f alloc] initWithFloatVector:[Vector3f xAxisPos]];
        up = [[Vector3f alloc] initWithFloatVector:[Vector3f zAxisPos]];
        right = [[Vector3f alloc] initWithFloatVector:[Vector3f yAxisNeg]];
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
        [right setFloat:[theCamera right]];

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

- (Vector3f *)right {
    return right;
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

- (void)lookAt:(Vector3f *)thePoint up:(Vector3f *)theUpVector {
    MathCache* cache = [MathCache sharedCache];
    Vector3f* d = [cache vector3f];
    
    [d setFloat:thePoint];
    [d sub:position];
    [self setDirection:d up:theUpVector];
    
    [cache returnVector3f:d];
}

- (void)setDirection:(Vector3f *)theDirection up:(Vector3f *)theUpVector {
    [direction setFloat:theDirection];
    [direction normalize];
    
    [up setFloat:theUpVector];
    [up normalize];
    
    [right setFloat:direction];
    [right cross:up];
    [right normalize];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)rotateYaw:(float)yaw pitch:(float)pitch {
    MathCache* cache = [MathCache sharedCache];
    
    Quaternion* qy = [cache quaternion];
    Quaternion* qp = [cache quaternion];
    [qy setAngle:yaw axis:[Vector3f zAxisPos]];
    [qp setAngle:pitch axis:right];
    [qy mul:qp];
    
    Vector3f* d = [cache vector3f];
    Vector3f* u = [cache vector3f];
    
    [d setFloat:direction];
    [qy rotate:d];
    
    [u setFloat:up];
    [qy rotate:u];
    
    if ([u z] < 0) {
        [u setZ:0];
        [d setX:0];
        [d setY:0];
    }
    
    [self setDirection:d up:u];
    
    [cache returnVector3f:d];
    [cache returnVector3f:u];
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
    
    [v setFloat:[Vector3f zAxisPos]];
    [v scale:u];
    [position add:v];

    [cache returnVector3f:v];

    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)orbitCenter:(Vector3f *)c hAngle:(float)h vAngle:(float)v {
    MathCache* cache = [MathCache sharedCache];
    
    Quaternion* qh = [cache quaternion];
    Quaternion* qv = [cache quaternion];
    [qv setAngle:v axis:right];
    [qh setAngle:h axis:[Vector3f zAxisPos]];
    [qh mul:qv];
    
    Vector3f* d = [cache vector3f];
    Vector3f* u = [cache vector3f];
    Vector3f* p = [cache vector3f];
    
    [d setFloat:direction];
    [qh rotate:d];
    
    [u setFloat:up];
    [qh rotate:u];
    
    [p setFloat:position];
    [p sub:c];
    
    if ([u z] < 0) {
        [u setFloat:up];

        [d setX:0];
        [d setY:0];
        [d normalize];
        
        Vector3f* axis = [cache vector3f];
        float angle = acos([direction dot:d]);
        if (angle != 0) {
            [axis setFloat:direction];
            [axis cross:d];
            [axis normalize];
            
            Quaternion* q = [cache quaternion];
            [q setAngle:angle axis:axis];
            [q rotate:p];
            [q rotate:u];
            
            [cache returnQuaternion:q];
        }
        
        [cache returnVector3f:axis];
    } else {
        [qh rotate:p];
    }
    
    [self setDirection:d up:u];
    
    [p add:c];
    [self moveTo:p];
    
    [cache returnVector3f:d];
    [cache returnVector3f:u];
    [cache returnVector3f:p];
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
    // glOrtho(NSWidth(bounds) / -2, NSWidth(bounds) / 2, NSHeight(bounds) / -2, NSHeight(bounds) / 2, near, far);
    
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
    gluUnProject(x, y, 0, modelview, projection, viewport, &rx, &ry, &rz);
    
    return [[[Vector3f alloc] initWithX:rx y:ry z:rz] autorelease];
}

- (Ray3D *)pickRayX:(float)x y:(float)y {
    Vector3f* rayDir = [self unprojectX:x y:y];
    
    [rayDir sub:position];
    [rayDir normalize];
    
    return [[[Ray3D alloc] initWithOrigin:position direction:rayDir] autorelease];

    /*
    Vector3f* rayDir = [[Vector3f alloc] initWithFloatVector:direction];
    Vector3f* rayPos = [self unprojectX:x y:y];
    return [[[Ray3D alloc] initWithOrigin:rayPos direction:[rayDir autorelease]] autorelease];
    */
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [position release];
    [direction release];
    [up release];
    [super dealloc];
}

@end
