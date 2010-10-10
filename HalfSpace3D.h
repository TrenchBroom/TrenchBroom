//
//  HalfSpace3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"
#import "Vector3i.h"
#import "Plane3D.h"

@interface HalfSpace3D : NSObject {
    Plane3D* boundary;
    Vector3f* outside;
}

+ (HalfSpace3D *)halfSpaceWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside;
+ (HalfSpace3D *)halfSpaceWithIntPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3;

- (id)initWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside;
- (id)initWithIntPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *) point3;

- (BOOL)contains:(Vector3f *)point;
- (Plane3D *)boundary;
@end
