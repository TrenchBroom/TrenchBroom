//
//  GridFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Figure.h"

@class Grid;
@class PickingHit;
@class Ray3D;

@interface GridFeedbackFigure : NSObject <Figure> {
    float gridPoints[2][2][3];
    int rows;
    int cols;
}

- (id)initWithGrid:(Grid *)grid pickingHit:(PickingHit *)pickingHit ray:(Ray3D *)ray;
@end
