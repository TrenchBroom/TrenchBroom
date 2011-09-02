//
//  PointFileFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.09.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "Figure.h"
#import "Math.h"

@interface PointFileFeedbackFigure : NSObject <Figure> {
    TVector3f* points;
    int pointCount;
}

- (id)initWithPoints:(TVector3f *)thePoints pointCount:(int)thePointCount;

@end
