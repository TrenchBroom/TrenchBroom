//
//  HalfSpace3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Plane3D;
@class Vector3f;
@class Vector3i;
@class Polygon3D;
@class Polyhedron;

@interface HalfSpace3D : NSObject {
    @private
    Plane3D* boundary;
    Vector3f* outside;
}

+ (HalfSpace3D *)halfSpaceWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside;
+ (HalfSpace3D *)halfSpaceWithIntPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3;

- (id)initWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside;
- (id)initWithIntPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *) point3;

- (BOOL)containsPoint:(Vector3f *)point;
- (BOOL)containsPolygon:(Polygon3D *)polygon;
- (BOOL)containsPolyhedron:(Polyhedron *)polyhedron;
- (Polygon3D *)intersectWithPolygon:(Polygon3D *)polygon;
- (Polyhedron *)intersectWithPolyhedron:(Polyhedron *)polyhedron;
- (Plane3D *)boundary;
@end
