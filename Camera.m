/*
Copyright (C) 2010-2012 Kristian Duske

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

#import "Camera.h"
#import "EditingSystem.h"
#import <OpenGL/glu.h>

NSString* const CameraChanged = @"CameraChanged";
NSString* const CameraViewChanged = @"CameraViewChanged";

@implementation Camera

- (id)init {
    if ((self = [super init])) {
        position = NullVector;
        position.x -= 64;
        position.y -= 64;
        position.z += 64;
        direction = XAxisPos;
        up = ZAxisPos;
        right = YAxisNeg;
        mode = CM_PERSPECTIVE;
        zoom = 0.5f;

        animations = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (id)initWithFieldOfVision:(float)theFov nearClippingPlane:(float)theNear farClippingPlane:(float)theFar {
    if ((self = [self init])) {
        fov = theFov;
        near = theNear;
        far = theFar;
    }
    
    return self;
}

- (id)initWithCamera:(Camera *)theCamera {
    if ((self = [self init])) {
        position = *[theCamera position];
        direction = *[theCamera direction];
        up = *[theCamera up];
        right = *[theCamera right];
        
        fov = [theCamera fieldOfVision];
        near = [theCamera nearClippingPlane];
        far = [theCamera farClippingPlane];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [animations release];
    [super dealloc];
}

- (const TVector3f *)position {
    return &position;
}

- (const TVector3f *)direction {
    return &direction;
}

- (const TVector3f *)up {
    return &up;
}

- (const TVector3f *)right {
    return &right;
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

- (float)zoom {
    return zoom;
}

- (ECameraMode)mode {
    return mode;
}

- (void)moveTo:(const TVector3f *)thePosition {
    NSAssert(!isnan(thePosition->x) && !isnan(thePosition->y) && !isnan(thePosition->z), @"position must be valid");

    if (equalV3f(&position, thePosition))
        return;
    
    position = *thePosition;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)lookAt:(const TVector3f *)thePoint up:(const TVector3f *)theUpVector {
    NSAssert(!isnan(thePoint->x) && !isnan(thePoint->y) && !isnan(thePoint->z), @"point must be valid");
    NSAssert(!isnan(theUpVector->x) && !isnan(theUpVector->y) && !isnan(theUpVector->z), @"up vector must be valid");

    TVector3f d;
    subV3f(thePoint, &position, &d);
    normalizeV3f(&d, &d);
    
    [self setDirection:&d up:theUpVector];
}

- (void)setDirection:(const TVector3f *)theDirection up:(const TVector3f *)theUpVector {
    NSAssert(!isnan(theDirection->x) && !isnan(theDirection->y) && !isnan(theDirection->z), @"direction must be valid");
    NSAssert(!isnan(theUpVector->x) && !isnan(theUpVector->y) && !isnan(theUpVector->z), @"up vector must be valid");

    if (equalV3f(theDirection, &direction) && equalV3f(theUpVector, &up))
        return;
    
    direction = *theDirection;

    crossV3f(&direction, theUpVector, &right);
    normalizeV3f(&right, &right);
    crossV3f(&right, &direction, &up);
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)rotateYaw:(float)yaw pitch:(float)pitch {
    if (yaw == 0 && pitch == 0)
        return;
    
    TQuaternion qy, qp;
    TVector3f d, u;
    
    setAngleAndAxisQ(&qy, yaw, &ZAxisPos);
    setAngleAndAxisQ(&qp, pitch, &right);
    mulQ(&qy, &qp, &qy);

    rotateQ(&qy, &direction, &d);
    rotateQ(&qy, &up, &u);
    
    if (u.z < 0) {
        u.z = 0;
        d.x = 0;
        d.y = 0;
    }
    
    [self setDirection:&d up:&u];
}

- (void)moveForward:(float)f right:(float)r up:(float)u {
    if (f == 0 && r == 0 && u == 0)
        return;
    
    TVector3f t;
    
    scaleV3f(&direction, f, &t);
    addV3f(&position, &t, &position);
    
    scaleV3f(&right, r, &t);
    addV3f(&position, &t, &position);

    scaleV3f(&up, u, &t);
    addV3f(&position, &t, &position);
    
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)orbitCenter:(const TVector3f *)c hAngle:(float)h vAngle:(float)v {
    if (h == 0 && v == 0)
        return;
    
    TQuaternion qv, qh;
    TVector3f d, u, p;
    
    setAngleAndAxisQ(&qv, v, &right);
    setAngleAndAxisQ(&qh, h, &ZAxisPos);
    mulQ(&qh, &qv, &qh);

    rotateQ(&qh, &direction, &d);
    rotateQ(&qh, &up, &u);
    subV3f(&position, c, &p);
    
    if (u.z < 0) {
        u = up;
        
        d.x = 0;
        d.y = 0;
        normalizeV3f(&d, &d);
        
        // correct rounding errors here
        float cos = fmaxf(-1, fminf(1, dotV3f(&direction, &d)));
        float angle = acos(cos);
        if (angle != 0) {
            TQuaternion q;
            TVector3f axis;

            crossV3f(&direction, &d, &axis);
            normalizeV3f(&axis, &axis);
            
            setAngleAndAxisQ(&q, angle, &axis);
            rotateQ(&q, &p, &p);
            rotateQ(&q, &u, &u);
        }
    } else {
        rotateQ(&qh, &p, &p);
    }
    
    [self setDirection:&d up:&u];
    
    addV3f(&p, c, &p);
    [self moveTo:&p];
}

- (void)setFieldOfVision:(float)theFov {
    if (fov == theFov)
        return;
    
    fov = theFov;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)setNearClippingPlane:(float)theNear {
    if (near == theNear)
        return;
    
    near = theNear;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)setFarClippingPlane:(float)theFar {
    if (far == theFar)
        return;
    
    far = theFar;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)setZoom:(float)theZoom {
    if (zoom == theZoom)
        return;
    
    zoom = theZoom;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)setMode:(ECameraMode)theMode {
    if (mode == theMode)
        return;
    
    mode = theMode;
    [[NSNotificationCenter defaultCenter] postNotificationName:CameraChanged object:self];
}

- (void)updateView:(NSRect)theViewport {
    BOOL changed = !NSEqualRects(viewport, theViewport);
    
    viewport = theViewport;
    glViewport(NSMinX(viewport), NSMinY(viewport), NSWidth(viewport), NSHeight(viewport));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (mode == CM_PERSPECTIVE)
        gluPerspective(fov, NSWidth(viewport) / NSHeight(viewport), near, far);
    else
        glOrtho(zoom * NSWidth(viewport) / -2, zoom * NSWidth(viewport) / 2, zoom * NSHeight(viewport) / -2, zoom * NSHeight(viewport) / 2, near, far);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(position.x,
              position.y,
              position.z,
              position.x + direction.x,
              position.y + direction.y,
              position.z + direction.z,
              up.x,
              up.y,
              up.z);

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    
    if (changed)
        [[NSNotificationCenter defaultCenter] postNotificationName:CameraViewChanged object:self];
}


- (TVector3f)unprojectX:(float)x y:(float)y depth:(float)depth {
    GLint viewportInt[] = {NSMinX(viewport), NSMinY(viewport), NSWidth(viewport), NSHeight(viewport)};
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    
    GLdouble rx, ry, rz;
    gluUnProject(x, y, depth, modelview, projection, viewportInt, &rx, &ry, &rz);
    
    TVector3f r = {rx, ry, rz};
    return r;
}

- (NSRect)viewport {
    return viewport;
}

- (TRay)pickRayX:(float)x y:(float)y {
    if (mode == CM_PERSPECTIVE) {
        TRay r;
        r.origin = position;
        r.direction = [self unprojectX:x y:y depth:0.5f];
        subV3f(&r.direction, &position, &r.direction);
        normalizeV3f(&r.direction, &r.direction);
        return r;
    } else {
        TRay r;
        r.origin = [self unprojectX:x y:y depth:0];
        r.direction = direction;
        return r;
    }
}

- (TVector3f)defaultPoint {
    TVector3f p;
    p = direction;
    scaleV3f(&p, 256, &p);
    addV3f(&p, &position, &p);
    return p;
}

- (TVector3f)defaultPointAtX:(float)x y:(float)y {
    TVector3f p = [self unprojectX:x y:y depth:0.5f];
    subV3f(&p, &position, &p);
    normalizeV3f(&p, &p);
    scaleV3f(&p, 256, &p);
    addV3f(&p, &position, &p);
    return p;
}

- (TVector3f)defaultPointOnRay:(const TRay *)ray {
    TVector3f point;
    rayPointAtDistance(ray, 256, &point);
    return point;
}

- (float)distanceTo:(const TVector3f *)thePoint {
    TVector3f diff;
    subV3f(thePoint, &position, &diff);
    return lengthV3f(&diff);
}

- (void)setBillboardMatrix {
    TVector3f bbLook, bbUp, bbRight;
    scaleV3f(&direction, -1, &bbLook);
    bbUp = up;
    crossV3f(&bbUp, &bbLook, &bbRight);
    
    /*
    subV3f(&position, theCenter, &bbLook);
    normalizeV3f(&bbLook, &bbLook);
    
    if (feq(bbLook.z, 1)) {
        bbUp = direction;
        scaleV3f(&bbUp, 1 / (1 - direction.z * direction.z), &bbUp); // project onto XY plane
        crossV3f(&bbUp, &bbLook, &bbRight);
    } else if (feq(bbLook.z, -1)) {
        scaleV3f(&bbUp, -1 / (1 - direction.z * direction.z), &bbUp); // project onto XY plane
        crossV3f(&bbUp, &bbLook, &bbRight);
    } else {
        crossV3f(&ZAxisPos, &bbLook, &bbRight);
        crossV3f(&bbLook, &bbRight, &bbUp);
    }
     */

    float matrix[] = {bbRight.x, bbRight.y, bbRight.z, 0, bbUp.x, bbUp.y, bbUp.z, 0, bbLook.x, bbLook.y, bbLook.z, 0, 0, 0, 0, 1};
    glMultMatrixf(matrix);
}

- (EditingSystem *)horizontalEditingSystem {
    return [[[EditingSystem alloc] initWithCamera:self vertical:NO] autorelease];
}

- (EditingSystem *)verticalEditingSystem {
    return [[[EditingSystem alloc] initWithCamera:self vertical:YES] autorelease];
}

- (NSMutableSet *)animations {
    return animations;
}

@end
