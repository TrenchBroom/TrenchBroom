//
//  ClipPlaneFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "FeedbackFigure.h"

@class Vector3i;

@interface ClipPlaneFeedbackFigure : NSObject <FeedbackFigure> {
    Vector3i* point1;
    Vector3i* point2;
    Vector3i* point3;
}

- (id)initWithPoint1:(Vector3i *)thePoint1 point2:(Vector3i *)thePoint2 point3:(Vector3i *)thePoint3;

@end
