//
//  GridFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "FeedbackFigure.h"

typedef enum {
    GO_XY,
    GO_YZ,
    GO_XZ
} EGridOrientation;

@class Grid;
@class BoundingBox;
@class Vector3f;

@interface GridFeedbackFigure : NSObject <FeedbackFigure> {
    float gridPoints[2][2][3];
    int rows;
    int cols;
}

- (id)initWithGrid:(Grid *)grid orientation:(EGridOrientation)orientation bounds:(BoundingBox *)bounds hitPoint:(Vector3f *)hitPoint;
@end
