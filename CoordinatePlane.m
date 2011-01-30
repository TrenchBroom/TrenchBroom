//
//  CoordinatePlane.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "CoordinatePlane.h"
#import "Vector2f.h"
#import "Vector3f.h"

static CoordinatePlane* gPlaneXY;
static CoordinatePlane* gPlaneXZ;
static CoordinatePlane* gPlaneYZ;

@implementation CoordinatePlane

+ (void)initialize {
    gPlaneXY = [[CoordinatePlane alloc] initAs:PXY];
    gPlaneXZ = [[CoordinatePlane alloc] initAs:PXZ];
    gPlaneYZ = [[CoordinatePlane alloc] initAs:PYZ];
}

+ (CoordinatePlane *)projectionPlaneForNormal:(Vector3f *)theNorm {
    if (theNorm == nil)
        [NSException raise:NSInvalidArgumentException format:@"normal must not be nil"];
    
    float x = fabs([theNorm x]);
    float y = fabs([theNorm y]);
    float z = fabs([theNorm z]);
    
    if (x >= y && x >= z)
        return gPlaneYZ;
    if (y >= x && y >= z)
        return gPlaneXZ;
    return gPlaneXY;
}

- (id)initAs:(EPlane3D)thePlane {
    if (self = [self init]) {
        plane = thePlane;
    }
    
    return self;
}

- (Vector2f *)project:(Vector3f *)thePoint {
    Vector2f* result = [[Vector2f alloc] init];
    switch (plane) {
        case PXY:
            [result setX:[thePoint x]];
            [result setY:[thePoint y]];
            break;
        case PXZ:
            [result setX:[thePoint x]];
            [result setY:[thePoint z]];
            break;
        case PYZ:
            [result setX:[thePoint y]];
            [result setY:[thePoint z]];
            break;
        default:
            break;
    }
    return [result autorelease];
}

@end
