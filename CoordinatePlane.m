//
//  CoordinatePlane.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "CoordinatePlane.h"
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
    
    EVectorComponent lc = [theNorm largestComponent];
    if (lc == VC_X)
        return gPlaneYZ;
    if (lc == VC_Y)
        return gPlaneXZ;
    return gPlaneXY;
}

- (id)initAs:(EPlane3D)thePlane {
    if (self = [self init]) {
        plane = thePlane;
    }
    
    return self;
}

- (BOOL)clockwise:(Vector3f *)theNorm {
    if (plane == PXY)
        return [theNorm z] < 0;
    if (plane == PXZ)
        return [theNorm y] < 0;
    return [theNorm x] > 0;
}

- (Vector3f *)project:(Vector3f *)thePoint {
    Vector3f* result = [[Vector3f alloc] init];
    switch (plane) {
        case PXY:
            [result setX:[thePoint x]];
            [result setY:[thePoint y]];
            [result setZ:[thePoint z]];
            break;
        case PXZ:
            [result setX:[thePoint x]];
            [result setY:[thePoint z]];
            [result setZ:[thePoint y]];
            break;
        case PYZ:
            [result setX:[thePoint y]];
            [result setY:[thePoint z]];
            [result setZ:[thePoint x]];
            break;
        default:
            break;
    }
    return [result autorelease];
}

- (float)xOf:(Vector3f *)thePoint {
    switch (plane) {
        case PXY:
            return [thePoint x];
        case PXZ:
            return [thePoint x];
        default:
            return [thePoint y];
    }
}

- (float)yOf:(Vector3f *)thePoint {
    switch (plane) {
        case PXY:
            return [thePoint y];
        case PXZ:
            return [thePoint z];
        default:
            return [thePoint z];
    }
}

- (float)zOf:(Vector3f *)thePoint {
    switch (plane) {
        case PXY:
            return [thePoint z];
        case PXZ:
            return [thePoint y];
        default:
            return [thePoint x];
    }
}

- (void)set:(Vector3f *)thePoint toX:(float)x y:(float)y z:(float)z {
    switch (plane) {
        case PXY:
            [thePoint setX:x];
            [thePoint setY:y];
            [thePoint setZ:z];
            break;
        case PXZ:
            [thePoint setX:x];
            [thePoint setZ:y];
            [thePoint setY:z];
            break;
        default:
            [thePoint setY:x];
            [thePoint setZ:y];
            [thePoint setX:z];
            break;
    }
}

@end
