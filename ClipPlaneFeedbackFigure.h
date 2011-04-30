//
//  ClipPlaneFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Figure.h"
#import "Math.h"

@interface ClipPlaneFeedbackFigure : NSObject <Figure> {
    TVector3i point1;
    TVector3i point2;
    TVector3i point3;
}

- (id)initWithPoint1:(TVector3i *)thePoint1 point2:(TVector3i *)thePoint2 point3:(TVector3i *)thePoint3;

@end
