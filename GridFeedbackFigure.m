//
//  GridFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GridFeedbackFigure.h"
#import "Grid.h"
#import "PickingHit.h"
#import "Face.h"
#import "Brush.h"

static int G = 2;

@implementation GridFeedbackFigure

- (id)initWithGrid:(Grid *)grid originalHit:(PickingHit *)originalHit ray:(TRay *)ray {
    NSAssert(grid != nil, @"grid must not be nil");
    NSAssert(originalHit != nil, @"hit must not be nil");
    NSAssert([originalHit type] == HT_FACE, @"hit type must be face");
    NSAssert(ray != NULL, @"ray must not be NULL");
    
    if ((self = [self init])) {
        id <Face> face = [originalHit object];
        id <Brush> brush = [face brush]; 
        
        TPlane* boundary = [face boundary];
        const TVector3f* originalPoint = [originalHit hitPoint];

        TVector3f hitPoint;
        float dist = intersectPlaneWithRay(boundary, ray);
        rayPointAtDistance(ray, dist, &hitPoint);

        TBoundingBox largeBounds = *[brush bounds];
        mergeBoundsWithPoint(&largeBounds, originalPoint, &largeBounds);
        mergeBoundsWithPoint(&largeBounds, &hitPoint, &largeBounds);
        [grid snapDownToGridV3f:&largeBounds.min result:&largeBounds.min];
        [grid snapUpToGridV3f:&largeBounds.max result:&largeBounds.max];
        expandBounds(&largeBounds, G * [grid actualSize], &largeBounds);
        
        TVector3f* min = &largeBounds.min;
        TVector3f* max = &largeBounds.max;
        TVector3f size;
        sizeOfBounds(&largeBounds, &size);
        
        switch (largestComponentV3f([face norm])) {
            case A_Z:
                cols = size.x / [grid actualSize];
                rows = size.y / [grid actualSize];
                
                gridPoints[0][0][0] = min->x;
                gridPoints[0][0][1] = min->y;
                gridPoints[0][0][2] = planeZ(boundary, min->x, min->y);
                gridPoints[0][1][0] = max->x;
                gridPoints[0][1][1] = min->y;
                gridPoints[0][1][2] = planeZ(boundary, max->x, min->y);
                gridPoints[1][0][0] = min->x;
                gridPoints[1][0][1] = max->y;
                gridPoints[1][0][2] = planeZ(boundary, min->x, max->y);
                gridPoints[1][1][0] = max->x;
                gridPoints[1][1][1] = max->y;
                gridPoints[1][1][2] = planeZ(boundary, max->x, max->y);
                break;
            case A_X:
                cols = size.y / [grid actualSize];
                rows = size.z / [grid actualSize];
                
                gridPoints[0][0][0] = planeX(boundary, min->y, min->z);
                gridPoints[0][0][1] = min->y;
                gridPoints[0][0][2] = min->z;
                gridPoints[0][1][0] = planeX(boundary, max->y, min->z);
                gridPoints[0][1][1] = max->y;
                gridPoints[0][1][2] = min->z;
                gridPoints[1][0][0] = planeX(boundary, min->y, max->z);
                gridPoints[1][0][1] = min->y;
                gridPoints[1][0][2] = max->z;
                gridPoints[1][1][0] = planeX(boundary, max->y, max->z);
                gridPoints[1][1][1] = max->y;
                gridPoints[1][1][2] = max->z;
                break;
            default:
                cols = size.x / [grid actualSize];
                rows = size.z / [grid actualSize];
                
                gridPoints[0][0][0] = min->x;
                gridPoints[0][0][1] = planeY(boundary, min->x, min->z);
                gridPoints[0][0][2] = min->z;
                gridPoints[0][1][0] = max->x;
                gridPoints[0][1][1] = planeY(boundary, max->x, min->z);
                gridPoints[0][1][2] = min->z;
                gridPoints[1][0][0] = min->x;
                gridPoints[1][0][1] = planeY(boundary, min->x, max->z);
                gridPoints[1][0][2] = max->z;
                gridPoints[1][1][0] = max->x;
                gridPoints[1][1][1] = planeY(boundary, max->x, max->z);
                gridPoints[1][1][2] = max->z;
                break;
        }
    }
    
    return self;
}

- (void)render {
    glColor4f(0, 1, 0, 0.2f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MAP2_VERTEX_3);
    glMap2f(GL_MAP2_VERTEX_3,
            0.0, 1.0,  /* U ranges 0..1 */
            3,         /* U stride, 3 floats per coord */
            2,         /* U is 2nd order, ie. linear */
            0.0, 1.0,  /* V ranges 0..1 */
            2 * 3,     /* V stride, row is 2 coords, 3 floats per coord */
            2,         /* V is 2nd order, ie linear */
            (const GLfloat*)gridPoints);  /* control points */
    glMapGrid2f(
                cols, 0.0, 1.0,
                rows, 0.0, 1.0);
    glEvalMesh2(GL_LINE,
                0, cols,   /* Starting at 0 mesh 5 steps (rows). */
                0, rows);  /* Starting at 0 mesh 6 steps (columns). */
    glDisable(GL_MAP2_VERTEX_3);
    glDisable(GL_DEPTH_TEST);
}

@end
