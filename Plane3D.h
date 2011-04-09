//
//  Plane3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    PS_ABOVE, // point is above the plane
    PS_BELOW, // point is below the plane
    PS_INSIDE // point is contained inside the plane
} EPointStatus;

@class Vector3f;
@class Vector3i;
@class Line3D;
@class Ray3D;

@interface Plane3D : NSObject {
    @private
    Vector3f* point;
    Vector3f* norm;
}

- (id)initWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm;
- (id)initWithPlane:(Plane3D *)aPlane;
- (id)initWithIntPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *) point3;

- (void)setPoint:(Vector3f *)thePoint norm:(Vector3f *)theNorm;

- (Vector3f *)point;
- (Vector3f *)norm;

- (EPointStatus)pointStatus:(Vector3f *)thePoint;

- (float)intersectWithLine:(Line3D *)line;
- (float)intersectWithRay:(Ray3D *)ray;

@end
