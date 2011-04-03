//
//  GridFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GridFeedbackFigure.h"
#import "Grid.h"
#import "Vector3f.h"
#import "BoundingBox.h"

static int G = 2;

@implementation GridFeedbackFigure

- (id)initWithGrid:(Grid *)grid orientation:(EGridOrientation)orientation bounds:(BoundingBox *)bounds hitPoint:(Vector3f *)hitPoint {
    NSAssert(grid != nil, @"grid must not be nil");
    NSAssert(bounds != nil, @"bounds must not be nil");
    NSAssert(hitPoint != nil, @"hit point must not be nil");
    
    if (self = [self init]) {
        Vector3f* min = [[Vector3f alloc] initWithFloatVector:[bounds min]];
        Vector3f* max = [[Vector3f alloc] initWithFloatVector:[bounds max]];
        [grid snapDownToGrid:min];
        [grid snapUpToGrid:max];
        
        Vector3f* size = [[Vector3f alloc] initWithFloatVector:max];
        [size sub:min];
        
        switch (orientation) {
            case GO_XY:
                cols = [size x] / [grid actualSize] + 2 * G;
                rows = [size y] / [grid actualSize] + 2 * G;
                
                gridPoints[0][0][0] = [min x] - G * [grid actualSize];
                gridPoints[0][0][1] = [min y] - G * [grid actualSize];
                gridPoints[0][0][2] = [hitPoint z];
                gridPoints[0][1][0] = [max x] + G * [grid actualSize];
                gridPoints[0][1][1] = [min y] - G * [grid actualSize];
                gridPoints[0][1][2] = [hitPoint z];
                gridPoints[1][0][0] = [min x] - G * [grid actualSize];
                gridPoints[1][0][1] = [max y] + G * [grid actualSize];
                gridPoints[1][0][2] = [hitPoint z];
                gridPoints[1][1][0] = [max x] + G * [grid actualSize];
                gridPoints[1][1][1] = [max y] + G * [grid actualSize];
                gridPoints[1][1][2] = [hitPoint z];
                break;
            case GO_YZ:
                cols = [size y] / [grid actualSize] + 2 * G;
                rows = [size z] / [grid actualSize] + 2 * G;
                
                gridPoints[0][0][0] = [hitPoint x];
                gridPoints[0][0][1] = [min y] - G * [grid actualSize];
                gridPoints[0][0][2] = [min z] - G * [grid actualSize];
                gridPoints[0][1][0] = [hitPoint x];
                gridPoints[0][1][1] = [max y] + G * [grid actualSize];
                gridPoints[0][1][2] = [min z] - G * [grid actualSize];
                gridPoints[1][0][0] = [hitPoint x];
                gridPoints[1][0][1] = [min y] - G * [grid actualSize];
                gridPoints[1][0][2] = [max z] + G * [grid actualSize];
                gridPoints[1][1][0] = [hitPoint x];
                gridPoints[1][1][1] = [max y] + G * [grid actualSize];
                gridPoints[1][1][2] = [max z] + G * [grid actualSize];
                break;
            default:
                cols = [size x] / [grid actualSize] + 2 * G;
                rows = [size z] / [grid actualSize] + 2 * G;
                
                gridPoints[0][0][0] = [min x] - G * [grid actualSize];
                gridPoints[0][0][1] = [hitPoint y];
                gridPoints[0][0][2] = [min z] - G * [grid actualSize];
                gridPoints[0][1][0] = [max x] + G * [grid actualSize];
                gridPoints[0][1][1] = [hitPoint y];
                gridPoints[0][1][2] = [min z] - G * [grid actualSize];
                gridPoints[1][0][0] = [min x] - G * [grid actualSize];
                gridPoints[1][0][1] = [hitPoint y];
                gridPoints[1][0][2] = [max z] + G * [grid actualSize];
                gridPoints[1][1][0] = [max x] + G * [grid actualSize];
                gridPoints[1][1][1] = [hitPoint y];
                gridPoints[1][1][2] = [max z] + G * [grid actualSize];
                break;
        }
        [min release];
        [max release];
    }
    
    return self;
}

- (void)render {
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x8888);
    glColor4f(1, 1, 1, 1);
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
    glDisable(GL_LINE_STIPPLE);
}

@end
