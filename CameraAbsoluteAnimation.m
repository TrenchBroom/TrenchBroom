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

#import "CameraAbsoluteAnimation.h"
#import "Camera.h"

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

@implementation CameraAbsoluteAnimation

- (id)initWithCamera:(Camera *)theCamera targetPosition:(const TVector3f *)thePosition targetLookAt:(const TVector3f *)theLookAt duration:(NSTimeInterval)duration {
    if ((self = [super initWithCamera:theCamera duration:duration])) {
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
                
                TVector3f inverseUp;
                scaleV3f(&initialUp, -1, &inverseUp);
                hAngle = horizontalAngle(&inverseUp, &targetDirection);
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
    }
    
    return self;
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
    
    normalizeV3f(&direction, &direction);
    normalizeV3f(&up, &up);
    
    [camera moveTo:&position];
    [camera setDirection:&direction up:&up];
}

@end
