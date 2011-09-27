//
//  GridFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Figure.h"
#import "Math.h"

@class Grid;
@class PickingHit;

@interface EditingPlaneFigure : NSObject <Figure> {
    Grid* grid;
    TPlane plane;
    GLfloat gridPoints[2][2][3];
    int rows;
    int cols;
}

- (id)initWithGrid:(Grid *)theGrid plane:(const TPlane *)thePlane bounds:(const TBoundingBox *)theBounds ray:(const TRay *)theRay;
- (void)setBounds:(const TBoundingBox *)theBounds ray:(const TRay *)theRay;

@end
