//
//  ControllerUtils.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ControllerUtils.h"
#import "Face.h"

BOOL calculateEntityOrigin(EntityDefinition* entityDefinition, PickingHitList* hits, NSPoint mousePos, Camera* camera, TVector3i* result) {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        const TVector3f* hitPoint = [hit hitPoint];
        const TBoundingBox* bounds = [entityDefinition bounds];
        TVector3f size;
        sizeOfBounds(bounds, &size);
        
        id <Face> face = [hit object];
        TVector3f* faceNorm = [face norm];
        
        EAxis lc = largestComponentV3f(faceNorm);
        BOOL d;
        switch (lc) {
            case A_X:
                if (faceNorm->x > 0) {
                    result->x = hitPoint->x - bounds->min.x;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = YES;
                } else {
                    result->x = hitPoint->x - size.x - bounds->min.x;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = NO;
                }
                break;
            case A_Y:
                if (faceNorm->y > 0) {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = YES;
                } else {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - size.y - bounds->min.y;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = NO;
                }
                break;
            case A_Z:
                if (faceNorm->z > 0) {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z;
                    d = YES;
                } else {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - size.z - bounds->min.z;
                    d = NO;
                }
                break;
        }
        
        return YES;
    } else {
        TVector3f posf = [camera defaultPointAtX:mousePos.x y:mousePos.y];
        roundV3f(&posf, result);
        return NO;
    }
}