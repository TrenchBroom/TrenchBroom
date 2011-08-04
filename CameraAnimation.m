//
//  CameraAnimator.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CameraAnimation.h"
#import "Camera.h"

static NSMutableSet* animations;

float horizontalAngle(const TVector3f* f, const TVector3f* t) {
    TVector3f fxy = *f;
    fxy.z = 0;
    normalizeV3f(&fxy, &fxy);
    TVector3f txy = *t;
    txy.z = 0;
    normalizeV3f(&txy, &txy);
    
    float n = acos(dotV3f(&fxy, &txy));
    TVector3f r;
    crossV3f(&fxy, &txy, &r);
    if (r.z > 0)
        return n;
    return -n;
}

@implementation CameraAnimation

+ (void)initialize {
    animations = [[NSMutableSet alloc] init];
}

- (id)initWithCamera:(Camera *)theCamera targetPosition:(const TVector3f *)thePosition targetLookAt:(TVector3f *)theLookAt duration:(NSTimeInterval)duration {
    if (self = [super initWithDuration:duration animationCurve:NSAnimationEaseInOut]) {
        camera = [theCamera retain];
        initialPosition = *[camera position];
        initialDirection = *[camera direction];
        initialUp = *[camera up];
        targetPosition = *thePosition;
        targetLookAt = *theLookAt;
        
        TVector3f targetDirection;
        subV3f(&targetLookAt, &targetPosition, &targetDirection);
        normalizeV3f(&targetDirection, &targetDirection);

        if (equalV3f(&initialDirection, &targetDirection)) {
            vAngle = 0;
            hAngle = 0;
        } else if (feq(initialDirection.z, 1)) {
            if (feq(targetDirection.z, -1)) {
                vAngle = -M_PI;
                hAngle = 0;
            } else {
                vAngle = -acos(dotV3f(&initialDirection, &targetDirection));
                hAngle = horizontalAngle(&initialUp, &targetDirection);
            }
        } else if (feq(initialDirection.z, -1)) {
            if (feq(targetDirection.z, 1)) {
                vAngle = M_PI;
                hAngle = 0;
            } else {
                vAngle = acos(dotV3f(&initialDirection, &targetDirection));
                hAngle = horizontalAngle(&initialUp, &targetDirection);
            }
        } else if (feq(targetDirection.z, 1)) {
            vAngle = acos(dotV3f(&initialDirection, &targetDirection));
            hAngle = 0;
        } else if (feq(targetDirection.z, -1)) {
            vAngle = -acos(dotV3f(&initialDirection, &targetDirection));
            hAngle = 0;
        } else {
            hAngle = horizontalAngle(&initialDirection, &targetDirection);
            
            TVector3f initialRotated;
            TQuaternion hRotation;
            setAngleAndAxisQ(&hRotation, hAngle, &ZAxisPos);
            rotateQ(&hRotation, &initialDirection, &initialRotated);
            
            vAngle = acos(dotV3f(&initialRotated, &targetDirection));
            if (initialRotated.z > targetDirection.z)
                vAngle *= -1;
        }

        /*
            
            
            
        } else {
            if (feq(initialDirection.z, 1)) {
                if (feq(targetDirection.z, -1)) {
                    vAngle = M_PI;
                } else {
                    vAngle = acos(dotV3f(&initialDirection, &targetDirection));
                }
                hAngle = 0;
            } else if (feq(initialDirection.z, -1)) {
                if (feq(targetDirection.z, -1)) {
                    vAngle = -M_PI;
                } else {
                    vAngle = acos(dotV3f(&initialDirection, &targetDirection));
                }
                hAngle = 0;
            } else {
                if (feq(targetDirection.z, 1)) {
                } else if (feq(targetDirection.z, -1)) {
                } else {
                }
            }
        }
         */
        
        TVector3f it, il, pn, cd;
        subV3f(&targetPosition, &initialPosition, &it);
        if (nullV3f(&it)) {
            positionCurve.start = initialPosition;
            positionCurve.startControl = positionCurve.start;
            positionCurve.end = targetPosition;
            positionCurve.endControl = positionCurve.end;
        } else {
            subV3f(&targetLookAt, &initialPosition, &il);
            crossV3f(&it, &il, &pn);
            if (nullV3f(&pn)) {
                TVector3f t;
                normalizeV3f(&it, &t);
                if(dotV3f(&t, &initialDirection) < dotV3f(&t, &initialUp)) {
                    crossV3f(&it, &initialDirection, &pn);
                } else {
                    crossV3f(&initialUp, &it, &pn);
                }
            }
            
            crossV3f(&it, &pn, &cd);
            normalizeV3f(&cd, &cd);
            scaleV3f(&cd, 0.5 * lengthV3f(&it), &cd);
            
            positionCurve.start = initialPosition;
            addV3f(&positionCurve.start, &cd, &positionCurve.startControl);
            addV3f(&positionCurve.startControl, &it, &positionCurve.endControl);
            positionCurve.end = targetPosition;
        }

        [super setFrameRate:60];
        [super setAnimationBlockingMode:NSAnimationNonblocking];
        [super setDelegate:self];
    }
    
    return self;
}

- (void)startAnimation {
    NSSet* copy = [[NSSet alloc] initWithSet:animations];
    NSEnumerator* animationEn = [copy objectEnumerator];
    NSAnimation* animation;
    while ((animation = [animationEn nextObject]))
        [animation stopAnimation];
    [copy release];
    
    [animations addObject:self];
    [super startAnimation];
}

- (void)setCurrentProgress:(NSAnimationProgress)progress {
    [super setCurrentProgress:progress];

    TVector3f position;
    pointOnCubicBezierCurve(&positionCurve, progress, &position);
    
    /*
    subV3f(&targetLookAt, &position, &intendedDirection);
    normalizeV3f(&intendedDirection, &intendedDirection);
    subV3f([camera direction], &intendedDirection, &error);
    scaleV3f(&error, progress, &error);
    subV3f([camera direction], &error, &direction);
    normalizeV3f(&direction, &direction);
    */

    TVector3f direction = initialDirection;
    TVector3f up = initialUp;
    TVector3f right;
    crossV3f(&direction, &up, &right);
    
    if (hAngle != 0) {
        TQuaternion hRotation;
        setAngleAndAxisQ(&hRotation, hAngle * progress, &ZAxisPos);
        rotateQ(&hRotation, &direction, &direction);
        rotateQ(&hRotation, &up, &up);
        rotateQ(&hRotation, &right, &right);
    }
    
    if (vAngle != 0) {
        TQuaternion vRotation;
        setAngleAndAxisQ(&vRotation, vAngle * progress, &right);
        rotateQ(&vRotation, &direction, &direction);
        rotateQ(&vRotation, &up, &up);
    }
    
    [camera moveTo:&position];
    [camera setDirection:&direction up:&up];
}

- (void)animationDidEnd:(NSAnimation *)animation {
    [animations removeObject:self];
    [self release];
}

- (void)animationDidStop:(NSAnimation *)animation {
    [animations removeObject:self];
    [self release];
}

- (void)animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress {
}

- (void)dealloc {
    [camera release];
    [super dealloc];
}

@end
