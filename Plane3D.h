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
@class Polygon3D;

@interface Plane3D : NSObject {
    Vector3f* point;
    Vector3f* norm;
}

+ (Plane3D *)planeWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm;
+ (Plane3D *)planeWithPlane:(Plane3D *)aPlane;

- (id)initWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm;
- (id)initWithPlane:(Plane3D *)aPlane;

- (Vector3f *)point;
- (Vector3f *)norm;

- (BOOL)isPointAbove:(Vector3f *)aPoint;

- (Vector3f *)intersectWithLine:(Line3D *)line;
- (Segment3D *)intersectWithPolygon:(Polygon3D *)polygon;

@end
