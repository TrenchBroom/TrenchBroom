//
//  Math3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Brush.h"

typedef enum {
    PXY, PXZ, PYZ
} Plane3D;

typedef enum {
    AXPos, AXNeg, AYPos, AYNeg, AZPos, AZNeg
} Axis3D;

int smallestVertex(NSArray *vertices);