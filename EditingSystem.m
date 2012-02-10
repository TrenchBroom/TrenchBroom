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

#import "EditingSystem.h"
#import "Camera.h"

@implementation EditingSystem

- (id)initWithCamera:(Camera *)theCamera vertical:(BOOL)vertical {
    NSAssert(theCamera != nil, @"camera must not be nil");
    
    if ((self = [self init])) {
        xAxisPos = firstAxisV3f([theCamera right]);
        xAxisNeg = thirdAxisV3f([theCamera right]);

        if (vertical) {
            yAxisPos = &ZAxisPos;
            yAxisNeg = &ZAxisNeg;
        } else {
            yAxisPos = firstAxisV3f([theCamera direction]);

            if (yAxisPos == &ZAxisPos || yAxisPos == &ZAxisNeg) {
                const TVector3f* up = [theCamera up];
                if (fabsf(up->x) > fabsf(up->y)) {
                    if (up->x > 0) {
                        yAxisPos = &XAxisPos;
                        yAxisNeg = &XAxisNeg;
                    } else {
                        yAxisPos = &XAxisNeg;
                        yAxisNeg = &XAxisPos;
                    }
                } else {
                    if (up->y > 0) {
                        yAxisPos = &YAxisPos;
                        yAxisNeg = &YAxisNeg;
                    } else {
                        yAxisPos = &YAxisNeg;
                        yAxisNeg = &YAxisPos;
                    }
                }
            } else {
                yAxisNeg = firstAxisNegV3f([theCamera direction]);
            }
        }

        
        TVector3f t;
        crossV3f(xAxisPos, yAxisPos, &t);
        zAxisPos = firstAxisV3f(&t);
        zAxisNeg = firstAxisNegV3f(&t);

        setColumnM4fV3f(&IdentityM4f, xAxisPos, 0, &worldToLocal);
        setColumnM4fV3f(&worldToLocal, yAxisPos, 1, &worldToLocal);
        setColumnM4fV3f(&worldToLocal, zAxisPos, 2, &worldToLocal);
        invertM4f(&worldToLocal, &localToWorld);
    }
    
    return self;
}

- (id)initWithCamera:(Camera *)theCamera yAxis:(const TVector3f*)theYAxis invert:(BOOL)invert {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theYAxis != NULL, @"Y axis must not be NULL");

    TVector3f t;

    if ((self = [self init])) {
        if (strongestComponentV3f(theYAxis) == A_Z) {
            xAxisPos = firstAxisV3f([theCamera right]);
            xAxisNeg = thirdAxisV3f([theCamera right]);

            if (invert) {
                yAxisPos = &ZAxisPos;
                yAxisNeg = &ZAxisNeg;
                
                crossV3f(xAxisPos, yAxisPos, &t);
                zAxisPos = firstAxisV3f(&t);
                zAxisNeg = thirdAxisV3f(&t);
            } else {
                zAxisPos = firstAxisV3f(theYAxis);
                zAxisNeg = thirdAxisV3f(theYAxis);
                
                crossV3f(zAxisPos, xAxisPos, &t);
                yAxisPos = firstAxisV3f(&t);
                yAxisNeg = thirdAxisV3f(&t);
            }
        } else {
            if (invert) {
                yAxisPos = firstAxisV3f(theYAxis);
                yAxisNeg = thirdAxisV3f(theYAxis);
                zAxisPos = &ZAxisPos;
                zAxisNeg = &ZAxisNeg;
            } else {
                yAxisPos = &ZAxisPos;
                yAxisNeg = &ZAxisNeg;
                zAxisPos = firstAxisV3f(theYAxis);
                zAxisNeg = thirdAxisV3f(theYAxis);
            }
            
            crossV3f(zAxisPos, yAxisPos, &t);
            xAxisPos = firstAxisV3f(&t);
            xAxisNeg = thirdAxisV3f(&t);
        }
    }
    
    return self;
}

- (const TVector3f *)xAxisPos {
    return xAxisPos;
}

- (const TVector3f *)xAxisNeg {
    return xAxisNeg;
}

- (const TVector3f *)yAxisPos {
    return yAxisPos;
}

- (const TVector3f *)yAxisNeg {
    return yAxisNeg;
}

- (const TVector3f *)zAxisPos {
    return zAxisPos;
}

- (const TVector3f *)zAxisNeg {
    return zAxisNeg;
}

- (float)intersectWithRay:(const TRay *)theRay planePosition:(const TVector3f *)thePlanePos {
    TPlane plane;
    plane.point = *thePlanePos;
    plane.norm = *zAxisPos;
    
    return intersectPlaneWithRay(&plane, theRay);
}

- (const TMatrix4f *)worldToLocalMatrix {
    return &worldToLocal;
}

- (const TMatrix4f *)localToWorldMatrix {
    return &localToWorld;
}

- (void)eliminateLRFrom:(const TVector3f *)theVector result:(TVector3f *)theResult {
    if (xAxisPos->x != 0) {
        theResult->x = 0;
        theResult->y = theVector->y;
        theResult->z = theVector->z;
    } else {
        theResult->x = theVector->x;
        theResult->y = 0;
        theResult->z = theVector->z;
    }
}

- (void)eliminateUDFrom:(const TVector3f *)theVector result:(TVector3f *)theResult {
    theResult->x = theVector->x;
    theResult->y = theVector->y;
    theResult->z = 0;
}

- (void)eliminateFBFrom:(const TVector3f *)theVector result:(TVector3f *)theResult {
    if (xAxisPos->x != 0) {
        theResult->x = theVector->x;
        theResult->y = 0;
        theResult->z = theVector->z;
    } else {
        theResult->x = 0;
        theResult->y = theVector->y;
        theResult->z = theVector->z;
    }
}

@end