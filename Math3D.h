//
//  Math3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Plane3D.h"
#import "Polygon3D.h"
#import "Polyhedron.h"
#import "Vector3f.h"
#import "Vector3i.h"
#import "HalfSpace3D.h"
#import "Line3D.h"

typedef enum {
    PXY, PXZ, PYZ
} Plane3D;

typedef enum {
    AXPos, AXNeg, AYPos, AYNeg, AZPos, AZNeg
} Axis3D;

@interface Math3D : NSObject {

}

@end
