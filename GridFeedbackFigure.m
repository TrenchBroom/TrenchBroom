//
//  GridFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GridFeedbackFigure.h"
#import "Vector3f.h"
#import "Grid.h"
#import "BoundingBox.h"
#import "PickingHit.h"
#import "Face.h"
#import "Brush.h"
#import "Plane3D.h"
#import "Ray3D.h"

static int G = 2;

@implementation GridFeedbackFigure

- (id)initWithGrid:(Grid *)grid pickingHit:(PickingHit *)pickingHit ray:(Ray3D *)ray {
    NSAssert(grid != nil, @"grid must not be nil");
    NSAssert(pickingHit != nil, @"hit must not be nil");
    NSAssert([pickingHit type] == HT_FACE, @"hit type must be face");
    
    if (self = [self init]) {
        id <Face> face = [pickingHit object];
        id <Brush> brush = [face brush]; 
        
        Plane3D* boundary = [face boundary];
        Vector3f* hitPoint = [ray pointAtDistance:[boundary intersectWithRay:ray]];

        BoundingBox* largeBounds = [[BoundingBox alloc] initWithBounds:[brush bounds]];
        [largeBounds mergePoint:hitPoint];
        [largeBounds expandToGrid:grid];
        [largeBounds expandBy:G * [grid actualSize]];
        
        Vector3f* min = [largeBounds min];
        Vector3f* max = [largeBounds max];
        Vector3f* size = [largeBounds size];
        
        switch ([[face norm] largestComponent]) {
            case VC_Z:
                cols = [size x] / [grid actualSize];
                rows = [size y] / [grid actualSize];
                
                gridPoints[0][0][0] = [min x];
                gridPoints[0][0][1] = [min y];
                gridPoints[0][0][2] = [boundary zAtX:[min x] y:[min y]];
                gridPoints[0][1][0] = [max x];
                gridPoints[0][1][1] = [min y];
                gridPoints[0][1][2] = [boundary zAtX:[max x] y:[min y]];
                gridPoints[1][0][0] = [min x];
                gridPoints[1][0][1] = [max y];
                gridPoints[1][0][2] = [boundary zAtX:[min x] y:[max y]];
                gridPoints[1][1][0] = [max x];
                gridPoints[1][1][1] = [max y];
                gridPoints[1][1][2] = [boundary zAtX:[max x] y:[max y]];
                break;
            case VC_X:
                cols = [size y] / [grid actualSize];
                rows = [size z] / [grid actualSize];
                
                gridPoints[0][0][0] = [boundary xAtY:[min y] z:[min z]];
                gridPoints[0][0][1] = [min y];
                gridPoints[0][0][2] = [min z];
                gridPoints[0][1][0] = [boundary xAtY:[max y] z:[min z]];
                gridPoints[0][1][1] = [max y];
                gridPoints[0][1][2] = [min z];
                gridPoints[1][0][0] = [boundary xAtY:[min y] z:[max z]];
                gridPoints[1][0][1] = [min y];
                gridPoints[1][0][2] = [max z];
                gridPoints[1][1][0] = [boundary xAtY:[max y] z:[max z]];
                gridPoints[1][1][1] = [max y];
                gridPoints[1][1][2] = [max z];
                break;
            default:
                cols = [size x] / [grid actualSize];
                rows = [size z] / [grid actualSize];
                
                gridPoints[0][0][0] = [min x];
                gridPoints[0][0][1] = [boundary yAtX:[min x] z:[min z]];
                gridPoints[0][0][2] = [min z];
                gridPoints[0][1][0] = [max x];
                gridPoints[0][1][1] = [boundary yAtX:[max x] z:[min z]];
                gridPoints[0][1][2] = [min z];
                gridPoints[1][0][0] = [min x];
                gridPoints[1][0][1] = [boundary yAtX:[min x] z:[max z]];
                gridPoints[1][0][2] = [max z];
                gridPoints[1][1][0] = [max x];
                gridPoints[1][1][1] = [boundary yAtX:[max x] z:[max z]];
                gridPoints[1][1][2] = [max z];
                break;
        }
        
        [largeBounds release];
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
            gridPoints);  /* control points */
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
