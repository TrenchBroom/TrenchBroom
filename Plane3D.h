//
//  Plane3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"
#import "Line3D.h"

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

- (Vector3f *)intersectWith:(Line3D *)line;

@end
