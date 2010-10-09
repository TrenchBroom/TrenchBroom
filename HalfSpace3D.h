//
//  HalfSpace3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"
#import "Plane3D.h"

@interface HalfSpace3D : NSObject {
    Plane3D* boundary;
    Vector3f* outside;
}

+ (HalfSpace3D *)halfSpaceWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside;

- (id)initWithBoundary:(Plane3D *)theBoundary outside:(Vector3f *)theOutside;

- (BOOL)contains:(Vector3f *)point;
- (Plane3D *)boundary;
@end
