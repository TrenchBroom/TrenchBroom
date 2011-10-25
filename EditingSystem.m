/*
Copyright (C) 2010-2011 Kristian Duske

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
        if (vertical) {
            xAxis = closestAxisV3f([theCamera right]);
            yAxis = &ZAxisPos;
            
            TVector3f t;
            crossV3f(xAxis, yAxis, &t);
            zAxis = closestAxisV3f(&t);
        } else {
            xAxis = closestAxisV3f([theCamera right]);
            yAxis = closestAxisV3f([theCamera direction]);
            if (yAxis == &ZAxisPos || yAxis == &ZAxisNeg)
                yAxis = closestAxisV3f([theCamera up]);
            
            zAxis = &ZAxisPos;
        }

        setColumnM4fV3f(&IdentityM4f, xAxis, 0, &worldToLocal);
        setColumnM4fV3f(&worldToLocal, yAxis, 1, &worldToLocal);
        setColumnM4fV3f(&worldToLocal, zAxis, 2, &worldToLocal);
        invertM4f(&worldToLocal, &localToWorld);
    }
    
    return self;
}

- (const TVector3f *)xAxis {
    return xAxis;
}

- (const TVector3f *)yAxis {
    return yAxis;
}

- (const TVector3f *)zAxis {
    return zAxis;
}

- (float)intersectWithRay:(const TRay *)theRay planePosition:(const TVector3f *)thePlanePos {
    TPlane plane;
    plane.point = *thePlanePos;
    plane.norm = *zAxis;
    
    return intersectPlaneWithRay(&plane, theRay);
}

- (const TMatrix4f *)worldToLocalMatrix {
    return &worldToLocal;
}

- (const TMatrix4f *)localToWorldMatrix {
    return &localToWorld;
}

- (void)eliminateLRFrom:(const TVector3f *)theVector result:(TVector3f *)theResult {
    if (xAxis->x != 0) {
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
    if (xAxis->x != 0) {
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