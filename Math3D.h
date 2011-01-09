//
//  Math3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    PXY, PXZ, PYZ
} EPlane3D;

typedef enum {
    AXPos, AXNeg, AYPos, AYNeg, AZPos, AZNeg
} EAxis3D;

int smallestVertex(NSArray *vertices);