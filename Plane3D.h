//
//  Plane3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;
@class Line3D;
@class Ray3D;
@class Polygon3D;

@interface Plane3D : NSObject {
    @private
    Vector3f* point;
    Vector3f* norm;
}

- (id)initWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm;
- (id)initWithPlane:(Plane3D *)aPlane;

- (void)setPoint:(Vector3f *)thePoint norm:(Vector3f *)theNorm;

- (Vector3f *)point;
- (Vector3f *)norm;

- (BOOL)isPointAbove:(Vector3f *)aPoint;

- (float)intersectWithLine:(Line3D *)line;
- (float)intersectWithRay:(Ray3D *)ray;

@end
